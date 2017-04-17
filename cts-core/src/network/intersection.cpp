#include <cts-core/base/log.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/intersection.h>
#include <cts-core/network/node.h>


namespace cts { namespace core
{


	Intersection::Intersection(const Connection& aConnection, double aTime, const Connection& bConnection, double bTime)
		: m_aConnection(&aConnection)
		, m_bConnection(&bConnection)
		, m_aTime(aTime)
		, m_bTime(bTime)
		, m_aArcPosition(m_aConnection->getCurve().timeToArcPosition(m_aTime))
		, m_bArcPosition(m_bConnection->getCurve().timeToArcPosition(m_bTime))
		, m_waitingDistance(0.0)
	{
		const double stepSize = 8.0;

		// compute waiting/clearance distance (= distance vehicles should wait in front in order to not block the other connection)
		// We do not determine this based on the angle of the intersection since the participating connections are not necessarily linear.
		double aArcPos = m_aArcPosition;
		double bArcPos = m_bArcPosition;
		double frontWaitingDistance = 0.0;
		while (math::distance(m_aConnection->getCurve().arcPositionToCoordinate(aArcPos), m_bConnection->getCurve().arcPositionToCoordinate(bArcPos)) < 22.0
			&& aArcPos > 0 && bArcPos > 0)
		{
			aArcPos -= stepSize;
			bArcPos -= stepSize;
			frontWaitingDistance += stepSize;
		}

		aArcPos = m_aArcPosition;
		bArcPos = m_bArcPosition;
		double rearWaitingDistance = 0.0;
		while (math::distance(m_aConnection->getCurve().arcPositionToCoordinate(aArcPos), m_bConnection->getCurve().arcPositionToCoordinate(bArcPos)) < 22.0
			&& aArcPos < m_aConnection->getCurve().getArcLength() && bArcPos < m_bConnection->getCurve().getArcLength())
		{
			aArcPos += stepSize;
			bArcPos += stepSize;
			rearWaitingDistance += stepSize;
		}

		m_waitingDistance = std::max(frontWaitingDistance, rearWaitingDistance);
	}


	bool Intersection::avoidBlocking() const
	{
		return (&m_aConnection->getStartNode() != &m_bConnection->getStartNode()) && (&m_aConnection->getEndNode() != &m_bConnection->getEndNode());
	}


	const Connection& Intersection::getFirstConnection() const
	{
		return *m_aConnection;
	}


	const Connection& Intersection::getSecondConnection() const
	{
		return *m_bConnection;
	}


	const Connection& Intersection::getOtherConnection(const Connection& connection) const
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		return (&connection == m_aConnection) ? *m_bConnection : *m_aConnection;

	}


	double Intersection::getFirstTime() const
	{
		return m_aTime;
	}


	double Intersection::getSecondTime() const
	{
		return m_bTime;
	}


	double Intersection::getMyTime(const Connection& connection) const
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		return (&connection == m_aConnection) ? m_aTime : m_bTime;
	}


	double Intersection::getFirstArcPosition() const
	{
		return m_aArcPosition;
	}


	double Intersection::getSecondArcPosition() const
	{
		return m_bArcPosition;
	}


	double Intersection::getMyArcPosition(const Connection& connection) const
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		return (&connection == m_aConnection) ? m_aArcPosition : m_bArcPosition;
	}


	vec2 Intersection::getFirstCoordinate() const
	{
		return m_aConnection->getCurve().timeToCoordinate(m_aTime);
	}


	vec2 Intersection::getSecondCoordinate() const
	{
		return m_bConnection->getCurve().timeToCoordinate(m_bTime);
	}


	double Intersection::getWaitingDistance() const
	{
		return m_waitingDistance;
	}


	void Intersection::registerVehicle(const AbstractVehicle& vehicle, const Connection& connection, double remainingDistance, vec2 blockingTime)
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);		
		auto& theMap = (&connection == m_aConnection) ? m_aCrossingVehicles : m_bCrossingVehicles;

		auto it = theMap.find(&vehicle);
		if (it != theMap.end())
		{
			// vehicle already registered, update CrossingVehicleInfo struct
			it->second.remainingDistance = remainingDistance;
			it->second.blockingTime = blockingTime;
			it->second.willWaitInFront = false;
		}
		else
		{
			theMap.emplace_hint(it, &vehicle, CrossingVehicleInfo{ blockingTime[0], remainingDistance, blockingTime, false });
		}
	}


	void Intersection::updateVehicleWait(const AbstractVehicle& vehicle, const Connection& connection, bool willWaitInFront)
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		auto& theMap = (&connection == m_aConnection) ? m_aCrossingVehicles : m_bCrossingVehicles;

		auto it = theMap.find(&vehicle);
		assert(it != theMap.end());

		if (it != theMap.end())
			it->second.willWaitInFront = willWaitInFront;
	}


	void Intersection::unregisterVehicle(const AbstractVehicle& vehicle, const Connection& connection)
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		auto& theMap = (&connection == m_aConnection) ? m_aCrossingVehicles : m_bCrossingVehicles;

		auto it = theMap.find(&vehicle);
		if (it != theMap.end())
			theMap.erase(it);
		else
			LOG_WARN("Intersection", "Trying to unregister unknown vehicle.");
	}


	std::vector<Intersection::CrossingVehicleInfo> Intersection::computeInterferingVehicles(const AbstractVehicle& vehicle, const Connection& connection)
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		auto& thisMap = (&connection == m_aConnection) ? m_aCrossingVehicles : m_bCrossingVehicles;
		auto& otherMap = (&connection == m_aConnection) ? m_bCrossingVehicles : m_aCrossingVehicles;

		assert(thisMap.find(&vehicle) != thisMap.end());
		auto thisCvt = thisMap[&vehicle];

		std::vector<CrossingVehicleInfo> toReturn;
		toReturn.reserve(otherMap.size());

		if (&m_aConnection->getStartNode() != &m_bConnection->getEndNode() || (m_waitingDistance < m_aArcPosition && m_waitingDistance < m_bArcPosition))
		{
			for (auto& it : otherMap)
			{
				auto& otherCvt = it.second;
				if ((!otherCvt.willWaitInFront || otherCvt.remainingDistance < 0)
					&& !(thisCvt.blockingTime[0] > otherCvt.blockingTime[1] || thisCvt.blockingTime[1] < otherCvt.blockingTime[0])) // computes intersectino of blocking time intervals
				{
					toReturn.push_back(otherCvt);
				}
			}
		}

		return toReturn;
	}


	Intersection::CrossingVehicleInfo& Intersection::getCrossingVehicleInfo(const AbstractVehicle& vehicle, const Connection& connection)
	{
		assert(&connection == m_aConnection || &connection == m_bConnection);
		auto& thisMap = (&connection == m_aConnection) ? m_aCrossingVehicles : m_bCrossingVehicles;

		assert(thisMap.find(&vehicle) != thisMap.end());
		return thisMap[&vehicle];
	}


}
}
