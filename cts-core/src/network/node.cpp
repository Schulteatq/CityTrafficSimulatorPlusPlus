
#include <cts-core/network/node.h>
#include <cts-core/network/connection.h>


namespace cts { namespace core
{


	Node::Node(const vec2& position)
		: m_position(position)
		, m_inSlope(0.0, 0.0)
		, m_outSlope(0.0, 0.0)
	{

	}


	Node::~Node()
	{
		s_deleted.emitSignal(this);
	}


	void Node::disconnect(Connection& /*connection*/)
	{

	}


	const vec2& Node::getPosition() const
	{
		return m_position;
	}


	void Node::setPosition(const vec2& value)
	{
		m_position = value;
		for (auto& connection : m_incomingConnections)
		{
			connection->updateCurve();
		}
		for (auto& connection : m_outgoingConnections)
		{
			connection->updateCurve();
		}
	}


	const vec2& Node::getInSlope() const
	{
		return m_inSlope;
	}


	void Node::setInSlope(const vec2& value)
	{
		m_inSlope = value;
		for (auto& connection : m_incomingConnections)
		{
			connection->updateCurve();
		}
	}


	const vec2& Node::getOutSlope() const
	{
		return m_outSlope;
	}


	void Node::setOutSlope(const vec2& value)
	{
		m_outSlope = value;
		for (auto& connection : m_outgoingConnections)
		{
			connection->updateCurve();
		}
	}


	const std::vector<Connection*>& Node::getIncomingConnections() const
	{
		return m_incomingConnections;
	}


	const std::vector<Connection*>& Node::getOutgoingConnections() const
	{
		return m_outgoingConnections;
	}


	Connection* Node::getConnectionTo(const Node& targetNode) const
	{
		for (auto& connection : m_outgoingConnections)
		{
			if (&connection->getEndNode() == &targetNode)
				return connection;
		}

		return nullptr;
	}


}
}
