#include <cts-core/network/connection.h>
#include <cts-core/traffic/vehicle.h>

#include <algorithm>
#include <cmath>

namespace cts { namespace core
{

	IdmMobil::IdmMobil()
		: m_safetyDistanceTime(1.4)
		, m_maximumAcceleration(1.2)
		, m_comfortDeceleration(1.5)
		, m_maximumDeceleration(3.0)
		, m_minimumDistance(20)
		, m_politeness(0.2)
		, m_lineChangingThreshold(0.75)
	{

	}
	

	double IdmMobil::getDesiredDistance(double velocity, double vDiff) const
	{
		// s* as defined in the IDM.
		double ss = m_minimumDistance + m_safetyDistanceTime * velocity + (velocity * vDiff) / (2 * sqrt(m_maximumAcceleration * m_comfortDeceleration));
		return std::max(ss, m_minimumDistance);
	}


	double IdmMobil::getAcceleration(double velocity, double desiredVelocity) const
	{
		return m_maximumAcceleration * (1.0 - pow(velocity / desiredVelocity, 2.0));
	}


	double IdmMobil::getAcceleration(double velocity, double desiredVelocity, double distance, double vDiff) const
	{
		double ss = getDesiredDistance(velocity, vDiff);
		return m_maximumAcceleration * (1.0 - pow(velocity / desiredVelocity, 2.0)) - sqrt(ss / distance);
	}


	// ================================================================================================


	AbstractVehicle::AbstractVehicle(const Node& start, const std::vector<Node*> destination, double targetVelocity)
		: m_targetVelocity(targetVelocity)
		, m_multiplierTargetVelocity(1.0)
		, m_acceleration(0.0)
		, m_velocity(targetVelocity)
		, m_routing()
		, m_currentConnection(nullptr)
		, m_destinationNodes(destination)
		, m_currentArcPosition(0.0)
		, m_length(40)
	{
		// FIXME: *this not fully constructed?!
		m_routing.compute(start, destination, *this);
	}


	double AbstractVehicle::getTargetVelocity() const
	{
		return m_targetVelocity;
	}


	double AbstractVehicle::getEffectiveTargetVelocity() const
	{
		return m_multiplierTargetVelocity * m_targetVelocity;
	}


	const cts::core::Connection* AbstractVehicle::getCurrentConnection() const
	{
		return m_currentConnection;
	}


	double AbstractVehicle::getCurrentArcPosition() const
	{
		return m_currentArcPosition;
	}


	double AbstractVehicle::getLength() const
	{
		return m_length;
	}


	double AbstractVehicle::think(Routing& routing, double arcPos) const
	{
		if (routing.getSegments().empty())
			return 0.0;

		// TODO...
		double minAcceleration = 0.0;
		return minAcceleration;
	}


	double AbstractVehicle::thinkOfVehiclesInFront(Routing& routing, double arcPos) const
	{
		static const double lookaheadDistance = 768.0;

		// Find the next vehicle in front of me
		// TODO: The original code also considers parallel connections if we're currently at the very beginning of our 
		// current connection. However, I would assume that this should also be covered by the intersection handling code.
		VehicleDistance vd = m_currentConnection->getVehicleBehind(m_currentArcPosition, lookaheadDistance);

		if (vd.empty())
		{
			return getAcceleration(m_velocity, getEffectiveTargetVelocity(), lookaheadDistance, m_velocity);
		}
		else
		{
			return getAcceleration(m_velocity, getEffectiveTargetVelocity(), vd.distance - vd.vehicle->getLength(), m_velocity - vd.vehicle->m_velocity);
		}
	}


}
}
