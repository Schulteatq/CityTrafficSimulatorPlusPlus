#ifndef CTS_CORE_NETWORK_H__
#define CTS_CORE_NETWORK_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/utils.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/intersection.h>
#include <cts-core/network/node.h>
#include <cts-core/traffic/trafficmanager.h>
#include <cts-core/traffic/vehicle.h>

#include <memory>
#include <vector>

namespace cts { namespace core
{

	class CTS_CORE_API Network : public utils::NotCopyable
	{
	public:
		using NodeListType = std::vector< std::unique_ptr<Node> >;
		using ConnectionListType = std::vector< std::unique_ptr<Connection> >;
		using VehicleListType = std::vector< std::unique_ptr<AbstractVehicle> >;
		using IntersectionListType = std::vector< std::unique_ptr<Intersection> >;


		Network();
		~Network();


		void importLegacyXml(const std::string& filename);

		Node* addNode(const vec2& position);
		void removeNode(Node& node);


		/// Creates a new Connection from \e startNode to \e endNode.
		/// Returns a nullptr if such a connection already exists.
		/// \param  startNode	Network node to connect from.
		/// \param  endNode		Network node to connect to.
		Connection* addConnection(Node& startNode, Node& endNode);

		/// Removes the given connection from the network.
		/// \param  connection	Connection to remove.
		void removeConnection(Connection& connection);


		TrafficManager& getTrafficManager();
		const NodeListType& getNodes() const;
		std::vector<Node*> getNodes(const Bounds2& bounds) const;
		
		std::vector< std::reference_wrapper<Connection> > getConnections() const;
		const VehicleListType& getVehicles() const;
		const IntersectionListType& getIntersections() const;

	private:
		IntersectionListType computeIntersections(Connection& connection, ConnectionListType::iterator start, ConnectionListType::iterator end, double tolerance);

		TrafficManager m_trafficMgr;

		NodeListType m_nodes;
		ConnectionListType m_connections;
		VehicleListType m_vehicles;
		IntersectionListType m_intersections;

		std::string m_title;
		std::string m_description;
	};

}
}

#endif
