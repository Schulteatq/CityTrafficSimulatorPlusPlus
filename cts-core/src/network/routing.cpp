#include <cts-core/base/utils.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/node.h>
#include <cts-core/network/routing.h>
#include <cts-core/traffic/vehicle.h>


#include <algorithm>
#include <memory>
#include <numeric>
#include <queue>
#include <set>

namespace cts { namespace core
{

	namespace
	{
		struct OpenListElement
		{
			OpenListElement(const Node& node, const std::shared_ptr<OpenListElement>& parent, int numParents, double previousCosts, double heuristicFullCosts)
				: node(node)
				, parent(parent)
				, numParents(numParents)
				, previousCosts(previousCosts)
				, heuristicFullCosts(heuristicFullCosts)
			{}

			const Node& node;							///< Node to investigate next
			std::shared_ptr<OpenListElement> parent;	///< Parent OpenListElement
			int numParents;								///< Number of parent elements
			double previousCosts;						///< Exact costs up to this element's node
			double heuristicFullCosts;					///< previousCosts + expected costs from element's node to the target node
		};

		static bool operator<(const std::shared_ptr<OpenListElement>& lhs, const std::shared_ptr<OpenListElement>& rhs)
		{
			return lhs->heuristicFullCosts > rhs->heuristicFullCosts;
		}

		template< class T, class Comp = std::less<T> >
		class RandomAccessPriorityQueue : public std::priority_queue<T, std::vector<T>, Comp>
		{
		public:
			RandomAccessPriorityQueue() = default;
			virtual ~RandomAccessPriorityQueue() = default;

			template<typename UnaryPredicate>
			void remove_erase_if(UnaryPredicate&& predicate)
			{
				utils::remove_erase_if(c, std::move(predicate));
				std::make_heap(c.begin(), c.end(), Comp());
			}
		};
	}


	const std::vector<Routing::Segment>& Routing::getSegments() const
	{
		return m_segments;
	}


	void Routing::compute(const Node& startNode, const std::vector<Node*>& destinationNodes, const AbstractVehicle& vehicle)
	{
		m_segments.clear();

		// TODO: move these constants somewhere more central where they make sense
		static const double VehicleOnRoutePenalty = 48.0;
		if (destinationNodes.empty())
			return;

		RandomAccessPriorityQueue< std::shared_ptr<OpenListElement> > openList;
		std::set<const Node*> closedList;

		openList.push(std::make_shared<OpenListElement>(startNode, nullptr, 0, 0.0, 0.0));
		do {
			std::shared_ptr<OpenListElement> ole = openList.top();
			openList.pop();

			// We found the shortest route, convert the OpenListElement into a list of routing segments
			if (utils::contains(destinationNodes, &ole->node))
			{
				m_segments.reserve(ole->numParents);
				for (OpenListElement* currentNode = ole.get(); currentNode->parent != nullptr; currentNode = currentNode->parent.get())
				{
					m_segments.push_back(Segment{ currentNode->parent->node.getConnectionTo(currentNode->node), &currentNode->parent->node, &currentNode->node });
				}

				std::reverse(m_segments.begin(), m_segments.end());
				return;
			}
			
			closedList.insert(&ole->node);
			for (auto& conn : ole->node.getOutgoingConnections())
			{
				// TODO: add check whether this vehicle is allowed to use the connection

				// check whether we have investigated this node already
				if (closedList.find(&conn->getEndNode()) != closedList.end()) 
					continue;

				// The following computation of the cost function is hand-crafted and taken from the original C# implementation of CTS...
				// Base costs are the the arc length of the connection
				double connectionCosts = conn->getCurve().getArcLength();
				// If the connection is congested, we induce a penalty, however only for the next two connections (otherwise the AI would not be able to know about that)
				if (ole->numParents < 3)
					connectionCosts += conn->getVehicles().size() * VehicleOnRoutePenalty;
				// consider the target velocity
				connectionCosts *= 14.0 / std::min(vehicle.getTargetVelocity(), conn->getTargetVelocity());
				
				const vec2 startPosition = conn->getEndNode().getPosition();
				const double remainingCosts = utils::reduce(destinationNodes, std::numeric_limits<double>::max(), [startPosition](double minimum, Node* node) {
					return std::min(minimum, math::distance(startPosition, node->getPosition()));
				});

				// check whether know already a better path to the end node of conn than the one we're currently examining.
				const double fullCosts = ole->previousCosts + connectionCosts + remainingCosts;
				bool nodeInOpenlist = false;

				openList.remove_erase_if([&](const std::shared_ptr<OpenListElement>& ole) {
					if (&ole->node == &conn->getEndNode())
					{
						if (ole->heuristicFullCosts < fullCosts)
						{
							nodeInOpenlist = true;
						}
						else
						{
							return true;
						}
					}
					return false;
				});

				if (!nodeInOpenlist)
					openList.push(std::make_shared<OpenListElement>(conn->getEndNode(), ole, ole->numParents + 1, ole->previousCosts + connectionCosts, fullCosts));
			}

		} while (!openList.empty());

	}


}
}
