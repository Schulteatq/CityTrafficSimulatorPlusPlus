#include <cts-core/base/utils.h>
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


	const Connection::VehicleListType& Connection::getVehicles() const
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


	void Connection::addVehicle(AbstractVehicle* vehicle, double arcPosition)
	{
		assert(std::find(m_vehicles.begin(), m_vehicles.end(), vehicle) == m_vehicles.end());

		auto it = vehicleIteratorBehind(arcPosition);
		m_vehicles.insert(it, vehicle);
	}


	void Connection::removeVehicle(AbstractVehicle* vehicle)
	{
		auto it = std::find(m_vehicles.begin(), m_vehicles.end(), vehicle);
		if (it != m_vehicles.end())
			m_vehicles.erase(it);
	}


	VehicleDistance Connection::getVehicleBehind(double arcPosition, double searchDistance) const
	{
		// check whether there is a vehicle on this connection
		auto it = vehicleIteratorBehind(arcPosition);
		if (it != m_vehicles.end())
		{
			return VehicleDistance(*it, (*it)->getCurrentArcPosition() - arcPosition);
		}
		// if not, check the following connections recursively
		else
		{
			const double remainingDistance = searchDistance - (m_curve.getArcLength() - arcPosition);
			if (remainingDistance <= 0.0)
			{
				return VehicleDistance();
			}
			else
			{
				VehicleDistance toReturn = utils::reduce(m_endNode.getOutgoingConnections(), VehicleDistance(), [remainingDistance](VehicleDistance lhs, Connection* c) {
					return VehicleDistance::min(lhs, c->getVehicleBehind(0, remainingDistance));
				});
				toReturn.distance += m_curve.getArcLength() - arcPosition;
				return toReturn;
			}
		}
	}


	VehicleDistance Connection::getVehicleBefore(double arcPosition, double searchDistance) const
	{
		// check whether there is a vehicle on this connection
		auto it = vehicleIteratorBefore(arcPosition);
		if (it != m_vehicles.end())
		{
			return VehicleDistance(*it, arcPosition - (*it)->getCurrentArcPosition());
		}
		// if not, check the following connections recursively
		else
		{
			const double remainingDistance = searchDistance - arcPosition;
			if (remainingDistance <= 0.0)
			{
				return VehicleDistance();
			}
			else
			{
				VehicleDistance toReturn = utils::reduce(m_startNode.getIncomingConnections(), VehicleDistance(), [remainingDistance](VehicleDistance lhs, Connection* c) {
					return VehicleDistance::min(lhs, c->getVehicleBefore(0, remainingDistance));
				});
				toReturn.distance += arcPosition;
				return toReturn;
			}
		}
	}


	const std::vector<Intersection*>& Connection::getIntersections() const
	{
		return m_intersections;
	}


	Connection::VehicleListType::const_iterator Connection::vehicleIteratorBehind(double arcPosition) const
	{
		return std::find_if(m_vehicles.cbegin(), m_vehicles.cend(), [arcPosition](AbstractVehicle* v) { return v->getCurrentArcPosition() > arcPosition; });
	}


	Connection::VehicleListType::const_iterator Connection::vehicleIteratorBefore(double arcPosition) const
	{
		auto rit = std::find_if(m_vehicles.crbegin(), m_vehicles.crend(), [arcPosition](AbstractVehicle* v) { return v->getCurrentArcPosition() < arcPosition; });
		if (rit == m_vehicles.crend())
			return m_vehicles.cend();
		else
			return (++rit).base();
	}


	void Connection::addIntersection(Intersection* intersection)
	{
		double insertArcPos = intersection->getMyArcPosition(*this);
		auto it = std::lower_bound(m_intersections.begin(), m_intersections.end(), insertArcPos, [this](Intersection* lhs, double value) {
			return lhs->getMyArcPosition(*this) < value;
		});
		m_intersections.insert(it, intersection);
	}


}
}
