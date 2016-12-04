#ifndef CTS_CORE_NODE_H__
#define CTS_CORE_NODE_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/math.h>

#include <memory>
#include <vector>

namespace cts { namespace core
{
	class Connection;
	class Network;

	class CTS_CORE_API Node
	{
	public:
		Node(const vec2& position);


		/// Creates a new Connection from this node to the target node.
		/// Returns a nullptr if such a connection already exists.
		/// \param  targetNode	Network node to connect to.
		std::unique_ptr<Connection> connectTo(Node& targetNode);


		/// Returns the world position of this node.
		const vec2& getPosition() const;
		/// Sets the world position of this node to \e value.
		void setPosition(const vec2& value);
				
		/// Returns the slope of incoming connections' Bézier curves (pointing toward the node).
		const vec2& getInSlope() const;
		/// Sets the slope of incoming connections' Bézier curves (pointing toward the node) to \e value.
		void setInSlope(const vec2& value);
		
		/// Returns the slope of outgoing connections' Bézier curves (pointing away from the node).
		const vec2& getOutSlope() const;
		/// Sets the slope of outgoing connections' Bézier curves to (pointing away from the node) \e value.
		void setOutSlope(const vec2& value);


		const std::vector<Connection*>& getIncomingConnections() const;

		const std::vector<Connection*>& getOutgoingConnections() const;

		Connection* getConnectionTo(const Node& targetNode) const;

	private:
		vec2 m_position;      ///< The world position of this node.
		vec2 m_inSlope;       ///< The slope of incoming connections' Bézier curves (pointing toward the node).
		vec2 m_outSlope;      ///< The slope of outgoing connections' Bézier curves (pointing away from the node).

		std::vector<Connection*> m_incomingConnections;
		std::vector<Connection*> m_outgoingConnections;

	};

}
}

#endif // CTS_NODE_H__