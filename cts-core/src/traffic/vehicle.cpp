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


	AbstractVehicle::AbstractVehicle()
		: m_targetVelocity(0.0)
		, m_multiplierTargetVelocity(1.0)
		, m_acceleration(0.0)
		, m_velocity(0.0)
		, m_currentConnection(nullptr)
		, m_currentArcPosition(0.0)
		, m_length(40)
	{

	}


	double AbstractVehicle::getTargetVelocity() const
	{
		return m_targetVelocity;
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


}
}
