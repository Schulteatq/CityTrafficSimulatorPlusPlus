
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


	std::unique_ptr<Connection> Node::connectTo(Node& targetNode)
	{
		if (getConnectionTo(targetNode) != nullptr)
			return nullptr;

		auto connection = std::make_unique<Connection>(*this, targetNode);
		m_outgoingConnections.push_back(connection.get());
		targetNode.m_incomingConnections.push_back(connection.get());
		return connection;
	}


	const vec2& Node::getPosition() const
	{
		return m_position;
	}


	void Node::setPosition(const vec2& value)
	{
		m_position = value;
	}


	const vec2& Node::getInSlope() const
	{
		return m_inSlope;
	}


	void Node::setInSlope(const vec2& value)
	{
		m_inSlope = value;
	}


	const vec2& Node::getOutSlope() const
	{
		return m_outSlope;
	}


	void Node::setOutSlope(const vec2& value)
	{
		m_outSlope = value;
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
