#include <cts-core/network/network.h>
#include <cts-core/base/log.h>
#include <cts-core/base/utils.h>

#include <tinyxml2.h>

#include <map>

namespace cts { namespace core
{
	Network::Network()
	{

	}


	Network::~Network()
	{
		m_trafficMgr.clearVehicles();
		m_vehicles.clear();
		m_intersections.clear();
	}


	void Network::importLegacyXml(const std::string& filename)
	{
		LOG_TRACE_GUARD("core.Network")

		// FIXME: this code lacks error checking at an insane level...
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());

		tinyxml2::XMLElement* rootNode = doc.FirstChildElement("CityTrafficSimulator");
		if (!rootNode || rootNode->IntAttribute("saveVersion") != 8)
			return;

		auto layoutNode = rootNode->FirstChildElement("Layout");
		if (!layoutNode)
			return;

		if (layoutNode->FirstChildElement("title")->GetText())
			m_title = layoutNode->FirstChildElement("title")->GetText();
		if (layoutNode->FirstChildElement("infoText")->GetText())
			m_description = layoutNode->FirstChildElement("infoText")->GetText();

		std::map<int, Node*> nodeHashes;
		for (tinyxml2::XMLElement* nodeElement = layoutNode->FirstChildElement("LineNode"); nodeElement != nullptr; nodeElement = nodeElement->NextSiblingElement("LineNode"))
		{
			const int hashCode = std::atoi(nodeElement->FirstChildElement("hashcode")->GetText());
			const double posX = std::atof(nodeElement->FirstChildElement("position")->FirstChildElement("X")->GetText());
			const double posY = std::atof(nodeElement->FirstChildElement("position")->FirstChildElement("Y")->GetText());
			const double inSlopeX = -std::atof(nodeElement->FirstChildElement("inSlope")->FirstChildElement("X")->GetText());
			const double inSlopeY = -std::atof(nodeElement->FirstChildElement("inSlope")->FirstChildElement("Y")->GetText());
			const double outSlopeX = std::atof(nodeElement->FirstChildElement("outSlope")->FirstChildElement("X")->GetText());
			const double outSlopeY = std::atof(nodeElement->FirstChildElement("outSlope")->FirstChildElement("Y")->GetText());

			Node* theNewNode = addNode(vec2(posX, posY));
			theNewNode->setInSlope(vec2(inSlopeX, inSlopeY));
			theNewNode->setOutSlope(vec2(outSlopeX, outSlopeY));
			nodeHashes[hashCode] = theNewNode;
		}


		for (tinyxml2::XMLElement* connectionElement = layoutNode->FirstChildElement("NodeConnection"); connectionElement != nullptr; connectionElement = connectionElement->NextSiblingElement("NodeConnection"))
		{
			const int startHash = std::atoi(connectionElement->FirstChildElement("startNodeHash")->GetText());
			const int endHash = std::atoi(connectionElement->FirstChildElement("endNodeHash")->GetText());
			const int priority = std::atoi(connectionElement->FirstChildElement("priority")->GetText());
			const double velocity = std::atof(connectionElement->FirstChildElement("targetVelocity")->GetText());

			auto startIt = nodeHashes.find(startHash);
			auto endIt = nodeHashes.find(endHash);
			if (startIt != nodeHashes.end() && endIt != nodeHashes.end())
			{
				auto theNewConnection = addConnection(*startIt->second, *endIt->second);
				if (theNewConnection)
				{
					theNewConnection->setPriority(priority);
					theNewConnection->setTargetVelocity(velocity);
				}
			}
		}


		for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
		{
			auto intersections = computeIntersections(**it, it, m_connections.end(), 4.0);
			m_intersections.insert(m_intersections.end(), std::make_move_iterator(intersections.begin()), std::make_move_iterator(intersections.end()));
		}


		auto tvNode = rootNode->FirstChildElement("TrafficVolumes");
		if (tvNode)
		{
			auto startNode = tvNode->FirstChildElement("StartPoints");
			auto endNode = tvNode->FirstChildElement("DestinationPoints");
			std::map<int, Location> startMap, destinationMap;

			for (tinyxml2::XMLElement* bonElement = startNode->FirstChildElement("BunchOfNodes"); bonElement != nullptr; bonElement = bonElement->NextSiblingElement("BunchOfNodes"))
			{
				const int hash = std::atoi(bonElement->FirstChildElement("hashcode")->GetText());
				const std::string title = bonElement->FirstChildElement("title")->GetText();
				std::vector<Node*> nodes;
				auto nhNode = bonElement->FirstChildElement("nodeHashes");
				for (tinyxml2::XMLElement* e = nhNode->FirstChildElement("int"); e != nullptr; e = e->NextSiblingElement("int"))
				{
					nodes.push_back(nodeHashes[std::atoi(e->GetText())]);
				}

				startMap.emplace(hash, Location(nodes, title));
			}

			for (tinyxml2::XMLElement* bonElement = endNode->FirstChildElement("BunchOfNodes"); bonElement != nullptr; bonElement = bonElement->NextSiblingElement("BunchOfNodes"))
			{
				const int hash = std::atoi(bonElement->FirstChildElement("hashcode")->GetText());
				const std::string title = bonElement->FirstChildElement("title")->GetText();
				std::vector<Node*> nodes;
				auto nhNode = bonElement->FirstChildElement("nodeHashes");
				for (tinyxml2::XMLElement* e = nhNode->FirstChildElement("int"); e != nullptr; e = e->NextSiblingElement("int"))
				{
					nodes.push_back(nodeHashes[std::atoi(e->GetText())]);
				}

				destinationMap.emplace(hash, Location(nodes, title));
			}

			for (tinyxml2::XMLElement* tvElement = tvNode->FirstChildElement("TrafficVolume"); tvElement != nullptr; tvElement = tvElement->NextSiblingElement("TrafficVolume"))
			{
				const int startHash = std::atoi(tvElement->FirstChildElement("startHash")->GetText());
				const int destinationHash = std::atoi(tvElement->FirstChildElement("destinationHash")->GetText());
				const int numCars = std::atoi(tvElement->FirstChildElement("trafficVolumeCars")->GetText());

				auto volume = m_trafficMgr.addVolume(startMap.find(startHash)->second.getNodes(), destinationMap.find(destinationHash)->second.getNodes());
				volume->carsPerHour = numCars;
			}
			int i = 0;
		}
	}


	Node* Network::addNode(const vec2& position)
	{
		m_nodes.push_back(std::make_unique<Node>(position));
		return m_nodes.back().get();
	}


	void Network::removeNode(Node& node)
	{
		while (!node.getIncomingConnections().empty())
		{
			removeConnection(*node.getIncomingConnections().front());
		}
		while (!node.getOutgoingConnections().empty())
		{
			removeConnection(*node.getOutgoingConnections().front());
		}

		utils::remove_erase_unique_ptr(m_nodes, &node);
	}


	Connection* Network::addConnection(Node& startNode, Node& endNode)
	{
		if (startNode.getConnectionTo(endNode) != nullptr)
			return nullptr;

		auto connection = std::make_unique<Connection>(startNode, endNode);
		startNode.m_outgoingConnections.push_back(connection.get());
		endNode.m_incomingConnections.push_back(connection.get());
		m_connections.push_back(std::move(connection));
		return m_connections.back().get();
	}


	void Network::removeConnection(Connection& connection)
	{
		utils::remove_erase(const_cast<Node&>(connection.m_startNode).m_outgoingConnections, &connection);
		utils::remove_erase(const_cast<Node&>(connection.m_endNode).m_incomingConnections, &connection);
		utils::remove_erase_unique_ptr(m_connections, &connection);
	}


	TrafficManager& Network::getTrafficManager()
	{
		return m_trafficMgr;
	}


	const std::vector< std::unique_ptr<Node> >& Network::getNodes() const
	{
		return m_nodes;
	}


	std::vector<Node*> Network::getNodes(const Bounds2& bounds) const
	{
		std::vector<Node*> toReturn;
		for (auto& node : m_nodes)
		{
			if (bounds.contains(node->getPosition()))
				toReturn.push_back(node.get());
		}
		return toReturn;
	}

	const Network::ConnectionListType& Network::getConnections() const
	{
		return m_connections;
	}


	const Network::VehicleListType& Network::getVehicles() const
	{
		return m_vehicles;
	}


	const Network::IntersectionListType& Network::getIntersections() const
	{
		return m_intersections;
	}


	namespace
	{
		struct ParameterizationInfo
		{
			const BezierParameterization* parameterization;
			double startTime;
			double endTime;
		};

		void computeIntersectionHelper(const Connection& lConn, const Connection& rConn, const ParameterizationInfo& lhs, const ParameterizationInfo& rhs, std::vector< std::pair<double, double> >& output, double tolerance)
		{
			const Bounds2& otherBounds = rhs.parameterization->getBounds();
			if (lhs.parameterization->getBounds().intersects(otherBounds))
			{
				double centerTime = rhs.startTime + (rhs.endTime - rhs.startTime) / 2.0;
				if (otherBounds.volume() > tolerance)
				{
					computeIntersectionHelper(lConn, rConn, lhs, { &rhs.parameterization->getSubdividedFirst(), rhs.startTime, centerTime }, output, tolerance);
					computeIntersectionHelper(lConn, rConn, lhs, { &rhs.parameterization->getSubdividedSecond(), centerTime, rhs.endTime }, output, tolerance);
				}
				else
				{
					output.emplace_back(lhs.startTime + (lhs.endTime - lhs.startTime), centerTime);
					//output.emplace_back(lConn, lhs.startTime + (lhs.endTime - lhs.startTime) / 2.0, rConn, centerTime);
				}
			}
		}
	}

	Network::IntersectionListType Network::computeIntersections(Connection& connection, ConnectionListType::iterator start, ConnectionListType::iterator end, double tolerance)
	{
		std::vector<ParameterizationInfo> bigParts{ { &connection.getCurve(), 0.0, 1.0 } };
		std::vector<ParameterizationInfo> smallParts;
		while (!bigParts.empty())
		{
			ParameterizationInfo part = bigParts.back();
			bigParts.pop_back();
			auto diagonal = part.parameterization->getBounds().getUrb() - part.parameterization->getBounds().getLlf();
			if (diagonal[0] > tolerance || diagonal[1] > tolerance)
			{
				bigParts.push_back({ &part.parameterization->getSubdividedFirst(), part.startTime, part.startTime + (part.endTime - part.startTime) / 2.0 });
				bigParts.push_back({ &part.parameterization->getSubdividedSecond(), part.startTime + (part.endTime - part.startTime) / 2.0, part.endTime });
			}
			else
			{
				smallParts.push_back(part);
			}
		}

		// Collect all possible intersections in terms of parameterization times
		IntersectionListType toReturn;
		std::vector< std::pair<double, double> > intersectionTimes;
		for (/**/; start != end; ++start)
		{
			auto& rConn = *start;
			if (rConn.get() == &connection)
				continue;

			intersectionTimes.clear();
			const auto& incomingConnections = connection.getStartNode().getIncomingConnections();
			const auto& outgoingConnections = connection.getEndNode().getOutgoingConnections();
			if (std::find(incomingConnections.begin(), incomingConnections.end(), rConn.get()) != incomingConnections.end())
				continue;
			if (std::find(outgoingConnections.begin(), outgoingConnections.end(), rConn.get()) != outgoingConnections.end())
				continue;

			ParameterizationInfo rPart{ &rConn->getCurve(), 0.0, 1.0 };
			for (auto& lPart : smallParts)
			{
				computeIntersectionHelper(connection, *rConn, lPart, rPart, intersectionTimes, tolerance);
			}

			if (intersectionTimes.empty())
				continue;
			

			// The following code does not work if we found only a single intersection.
			// Instead of making it more complicated to support such scenarios, we simply duplicate the single found intersection.
			if (intersectionTimes.size() == 1)
				intersectionTimes.push_back(intersectionTimes[0]);

			// merge intersections that are very close to each other
			std::sort(intersectionTimes.begin(), intersectionTimes.end(), [](const std::pair<double, double>& lhs, const std::pair<double, double>& rhs) {
				return lhs.first < rhs.first;
			});

			size_t startIndex = 0;
			double startArcPos = connection.getCurve().timeToArcPosition(intersectionTimes[0].first);
			double lastArcPos = startArcPos;
			for (size_t i = 0; i < intersectionTimes.size(); ++i)
			{
				const double currentArcPos = connection.getCurve().timeToArcPosition(intersectionTimes[i].first);
				if (currentArcPos - lastArcPos > 42 || i+1 == intersectionTimes.size()) // FIXME: make constant configurable
				{
					toReturn.emplace_back(new Intersection(connection, intersectionTimes[startIndex].first + (intersectionTimes[i-1].first - intersectionTimes[startIndex].first) / 2.0, *rConn, intersectionTimes[startIndex + (i - 1 - startIndex) / 2].second));
					connection.addIntersection(toReturn.back().get());
					rConn->addIntersection(toReturn.back().get());

					startIndex = i;
					startArcPos = currentArcPos;
					lastArcPos = currentArcPos;
				}
				else
				{
					lastArcPos = currentArcPos;
				}
			}

		}

		return toReturn;
	}


}
}
