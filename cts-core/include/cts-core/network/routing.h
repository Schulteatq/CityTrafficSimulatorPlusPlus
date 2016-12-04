#ifndef CTS_CORE_ROUTING_H__
#define CTS_CORE_ROUTING_H__

#include <cts-core/coreapi.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/node.h>
#include <cts-core/traffic/vehicle.h>

#include <memory>
#include <vector>

namespace cts { namespace core
{

	class CTS_CORE_API Routing
	{
	public:
		struct Segment
		{
			const Node* startNode;
			const Node* targetNode;
		};

		Routing(const Node& startNode, const Node& targetNode, const AbstractVehicle& vehicle);

		const std::vector<Segment>& getSegments() const;

	private:
		void compute(const Node& startNode, const Node& targetNode, const AbstractVehicle& vehicle);

		std::vector<Segment> m_segments;

	};

}
}

#endif // CTS_NETWORK_H__
