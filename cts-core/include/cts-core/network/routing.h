#ifndef CTS_CORE_ROUTING_H__
#define CTS_CORE_ROUTING_H__

#include <cts-core/coreapi.h>

#include <vector>

namespace cts { namespace core
{
	class AbstractVehicle;
	class Connection;
	class Node;

	class CTS_CORE_API Routing
	{
	public:
		struct Segment
		{
			const Connection* connection;
			const Node* start;
			const Node* destination;
		};

		Routing() = default;

		void compute(const Node& startNode, const std::vector<Node*>& destinationNodes, const AbstractVehicle& vehicle);
		const std::vector<Segment>& getSegments() const;

	private:

		std::vector<Segment> m_segments;

	};

}
}

#endif // CTS_NETWORK_H__
