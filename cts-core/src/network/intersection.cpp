#include <cts-core/network/intersection.h>


namespace cts { namespace core
{


	Intersection::Intersection(const Connection& aConnection, double aTime, const Connection& bConnection, double bTime)
		: m_aConnection(&aConnection)
		, m_bConnection(&bConnection)
		, m_aTime(aTime)
		, m_bTime(bTime)
		, m_aArcPosition(m_aConnection->getParameterization().timeToArcPosition(m_aTime))
		, m_bArcPosition(m_bConnection->getParameterization().timeToArcPosition(m_bTime))
		, m_waitingDistance(0.0)
	{
		const double stepSize = 8.0;

		// compute waiting/clearance distance (= distance vehicles should wait in front in order to not block the other connection)
		// We do not determine this based on the angle of the intersection since the participating connections are not necessarily linear.
		double aArcPos = m_aArcPosition;
		double bArcPos = m_bArcPosition;
		double frontWaitingDistance = 0.0;
		while (math::distance(m_aConnection->getParameterization().arcPositionToCoordinate(aArcPos), m_bConnection->getParameterization().arcPositionToCoordinate(bArcPos)) < 22.0
			&& aArcPos > 0 && bArcPos > 0)
		{
			aArcPos -= stepSize;
			bArcPos -= stepSize;
			frontWaitingDistance += stepSize;
		}

		aArcPos = m_aArcPosition;
		bArcPos = m_bArcPosition;
		double rearWaitingDistance = 0.0;
		while (math::distance(m_aConnection->getParameterization().arcPositionToCoordinate(aArcPos), m_bConnection->getParameterization().arcPositionToCoordinate(bArcPos)) < 22.0
			&& aArcPos < m_aConnection->getParameterization().getArcLength() && bArcPos < m_bConnection->getParameterization().getArcLength())
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


	double Intersection::getFirstTime() const
	{
		return m_aTime;
	}


	double Intersection::getSecondTime() const
	{
		return m_bTime;
	}


	double Intersection::getFirstArcPosition() const
	{
		return m_aArcPosition;
	}


	double Intersection::getSecondArcPosition() const
	{
		return m_bArcPosition;
	}


	vec2 Intersection::getFirstCoordinate() const
	{
		return m_aConnection->getParameterization().timeToCoordinate(m_aTime);
	}


	vec2 Intersection::getSecondCoordinate() const
	{
		return m_bConnection->getParameterization().timeToCoordinate(m_bTime);
	}


	double Intersection::getWaitingDistance() const
	{
		return m_waitingDistance;
	}


}
}
