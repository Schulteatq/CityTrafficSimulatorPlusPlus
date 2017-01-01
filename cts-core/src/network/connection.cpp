#include <cts-core/network/connection.h>
#include <cts-core/network/node.h>
#include <cts-core/traffic/vehicle.h>

#include <algorithm>

namespace cts { namespace core
{

	Connection::Connection(const Node& startNode, const Node& endNode)
		: m_startNode(startNode)
		, m_endNode(endNode)
		, m_curve(startNode.getPosition(), startNode.getPosition() + startNode.getOutSlope(), endNode.getPosition() - endNode.getInSlope(), endNode.getPosition())
		, m_priority(1)
		, m_targetVelocity(10.0)
	{

	}

	
	const Node& Connection::getStartNode() const
	{
		return m_startNode;
	}


	const Node& Connection::getEndNode() const
	{
		return m_endNode;
	}


	const BezierParameterization& Connection::getCurve() const
	{
		return m_curve;
	}


	const std::list<AbstractVehicle*>& Connection::getVehicles() const
	{
		return m_vehicles;
	}


	int Connection::getPriority() const
	{
		return m_priority;
	}


	void Connection::setPriority(int value)
	{
		m_priority = value;
	}


	double Connection::getTargetVelocity() const
	{
		return m_targetVelocity;
	}


	void Connection::setTargetVelocity(double value)
	{
		m_targetVelocity = value;
	}


	void Connection::updateCurve()
	{
		m_curve = BezierParameterization(m_startNode.getPosition(), m_startNode.getPosition() + m_startNode.getOutSlope(), m_endNode.getPosition() - m_endNode.getInSlope(), m_endNode.getPosition());
	}

	std::list<AbstractVehicle*>::const_iterator Connection::findVehicleBehind(double arcPosition) const
	{
		return std::find_if(m_vehicles.cbegin(), m_vehicles.cend(), [arcPosition](AbstractVehicle* v) { return v->getCurrentArcPosition() > arcPosition; });
	}


	std::list<AbstractVehicle*>::const_iterator Connection::findVehicleBefore(double arcPosition) const
	{
		auto rit = std::find_if(m_vehicles.crbegin(), m_vehicles.crend(), [arcPosition](AbstractVehicle* v) { return v->getCurrentArcPosition() < arcPosition; });
		if (rit == m_vehicles.crend())
			return m_vehicles.cend();
		else
			return (++rit).base();
	}


}
}
