#ifndef CTS_CORE_NETWORK_H__
#define CTS_CORE_NETWORK_H__

#include <cts-core/coreapi.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/intersection.h>
#include <cts-core/network/node.h>
#include <cts-core/traffic/vehicle.h>

#include <memory>
#include <vector>

namespace cts { namespace core
{

	class CTS_CORE_API Network
	{
	public:
		Network();

		~Network();

		Network(const Network&) = delete;
		Network& operator=(const Network&) = delete;


		void importLegacyXml(const std::string& filename);

		Node* addNode(const vec2& position);

		Connection* addConnection(Node& startNode, Node& endNode);


		const std::vector< std::unique_ptr<Node> >& getNodes() const;
		const std::vector< std::unique_ptr<Connection> >& getConnections() const;
		const std::vector< std::unique_ptr<AbstractVehicle> >& getVehicles() const;
		const std::vector<Intersection>& getIntersections() const;

	private:
		std::vector<Intersection> computeIntersections(Connection& connection, double tolerance);

		std::vector< std::unique_ptr<Node> > m_nodes;
		std::vector< std::unique_ptr<Connection> > m_connections;
		std::vector< std::unique_ptr<AbstractVehicle> > m_vehicles;

		std::vector<Intersection> m_intersections;

		std::string m_title;
		std::string m_description;
	};

}
}

#endif
