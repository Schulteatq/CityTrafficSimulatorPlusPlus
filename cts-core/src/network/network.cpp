#include <cts-core/network/network.h>

#include <tinyxml2.h>

#include <map>

namespace cts { namespace core
{
	Network::Network()
	{

	}


	Network::~Network()
	{

	}


	void Network::importLegacyXml(const std::string& filename)
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());

		tinyxml2::XMLElement* rootNode = doc.FirstChildElement("CityTrafficSimulator");
		if (!rootNode || rootNode->IntAttribute("saveVersion") != 8)
			return;

		auto layoutNode = rootNode->FirstChildElement("Layout");
		if (!layoutNode)
			return;

		m_title = layoutNode->FirstChildElement("title")->GetText();
		m_description = layoutNode->FirstChildElement("infoText")->GetText();

		std::map<int, Node*> nodeHashes;
		for (tinyxml2::XMLElement* nodeElement = layoutNode->FirstChildElement("LineNode"); nodeElement != nullptr; nodeElement = nodeElement->NextSiblingElement("LineNode"))
		{
			int hashCode = std::atoi(nodeElement->FirstChildElement("hashcode")->GetText());
			double posX = std::atof(nodeElement->FirstChildElement("position")->FirstChildElement("X")->GetText());
			double posY = std::atof(nodeElement->FirstChildElement("position")->FirstChildElement("Y")->GetText());
			double inSlopeX = std::atof(nodeElement->FirstChildElement("inSlope")->FirstChildElement("X")->GetText());
			double inSlopeY = std::atof(nodeElement->FirstChildElement("inSlope")->FirstChildElement("Y")->GetText());
			double outSlopeX = std::atof(nodeElement->FirstChildElement("outSlope")->FirstChildElement("X")->GetText());
			double outSlopeY = std::atof(nodeElement->FirstChildElement("outSlope")->FirstChildElement("Y")->GetText());

			Node* theNewNode = addNode(vec2(posX, posY));
			theNewNode->setInSlope(vec2(inSlopeX, inSlopeY));
			theNewNode->setOutSlope(vec2(outSlopeX, outSlopeY));
			nodeHashes[hashCode] = theNewNode;
		}


		for (tinyxml2::XMLElement* connectionElement = layoutNode->FirstChildElement("NodeConnection"); connectionElement != nullptr; connectionElement = connectionElement->NextSiblingElement("NodeConnection"))
		{
			int startHash = std::atoi(connectionElement->FirstChildElement("startNodeHash")->GetText());
			int endHash = std::atoi(connectionElement->FirstChildElement("endNodeHash")->GetText());
			int priority = std::atoi(connectionElement->FirstChildElement("priority")->GetText());
			double velocity = std::atof(connectionElement->FirstChildElement("targetVelocity")->GetText());

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


		for (auto& connection : m_connections)
		{
			auto ints = computeIntersections(*connection, 4);
			m_intersections.insert(m_intersections.end(), ints.begin(), ints.end());
		}
	}


	Node* Network::addNode(const vec2& position)
	{
		m_nodes.push_back(std::make_unique<Node>(position));
		return m_nodes.back().get();
	}


	Connection* Network::addConnection(Node& startNode, Node& endNode)
	{
		auto connection = startNode.connectTo(endNode);
		if (connection == nullptr)
			return nullptr;

		m_connections.push_back(std::move(connection));
		return m_connections.back().get();
	}


	const std::vector< std::unique_ptr<Node> >& Network::getNodes() const
	{
		return m_nodes;
	}


	const std::vector< std::unique_ptr<Connection> >& Network::getConnections() const
	{
		return m_connections;
	}


	const std::vector< std::unique_ptr<AbstractVehicle> >& Network::getVehicles() const
	{
		return m_vehicles;
	}


	const std::vector<cts::core::Intersection>& Network::getIntersections() const
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

		void computeIntersectionHelper(const Connection& lConn, const Connection& rConn, const ParameterizationInfo& lhs, const ParameterizationInfo& rhs, std::vector<Intersection>& output, double tolerance)
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
					output.emplace_back(lConn, lhs.startTime + (lhs.endTime - lhs.startTime) / 2.0, rConn, centerTime);
				}
			}
		}
	}

	std::vector<Intersection> Network::computeIntersections(Connection& connection, double tolerance)
	{
		std::vector<ParameterizationInfo> bigParts{ { &connection.getParameterization(), 0.0, 1.0 } };
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

		std::vector<Intersection> toReturn;
		for (auto& rConn : m_connections)
		{
			if (rConn.get() == &connection)
				continue;

			auto incomingConnections = connection.getStartNode().getIncomingConnections();
			auto outgoingConnections = connection.getEndNode().getOutgoingConnections();
			if (std::find(incomingConnections.begin(), incomingConnections.end(), rConn.get()) != incomingConnections.end())
				continue;
			if (std::find(outgoingConnections.begin(), outgoingConnections.end(), rConn.get()) != outgoingConnections.end())
				continue;

			ParameterizationInfo rPart{ &rConn->getParameterization(), 0.0, 1.0 };
			for (auto& lPart : smallParts)
			{
				computeIntersectionHelper(connection, *rConn, lPart, rPart, toReturn, tolerance);
			}
		}
		return toReturn;
	}


}
}
