#include <cts-core/base/utils.h>
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
		return m_maximumAcceleration * (1.0 - pow(velocity / desiredVelocity, 2.0) - sqrt(ss / distance));
	}


	// ================================================================================================


	const double AbstractVehicle::m_lookaheadDistance = 768.0;


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
		static int counter = 0;
		debugId = ++counter;

		// FIXME: *this not fully constructed?!
		updateRouting(start, destination);

		if (!m_routing.getSegments().empty())
		{
			setCurrentConnection(m_routing.getSegments()[0].connection);
			m_visitedConnections.reserve(m_routing.getSegments().size());
		}
	}


	double AbstractVehicle::getTargetVelocity() const
	{
		return m_targetVelocity;
	}


	double AbstractVehicle::getEffectiveTargetVelocity() const
	{
		return std::min(m_multiplierTargetVelocity * m_targetVelocity, m_currentConnection->getTargetVelocity());
	}


	const cts::core::Connection* AbstractVehicle::getCurrentConnection() const
	{
		return m_currentConnection;
	}


	void AbstractVehicle::setCurrentConnection(const Connection* value)
	{
		if (m_currentConnection != nullptr)
			const_cast<Connection*>(m_currentConnection)->removeVehicle(this);

		m_currentConnection = value;

		if (m_currentConnection != nullptr)
			const_cast<Connection*>(m_currentConnection)->addVehicle(this, m_currentArcPosition);
	}


	double AbstractVehicle::getCurrentArcPosition() const
	{
		return m_currentArcPosition;
	}


	void AbstractVehicle::setCurrentArcPosition(double value)
	{
		m_currentArcPosition = value;
	}


	double AbstractVehicle::getLength() const
	{
		return m_length;
	}


	void AbstractVehicle::prepare(double currentTime)
	{
		auto tailIt = m_registeredIntersections.begin(); // pointer to the first SpecificIntersection behind the vehicle's tail
		auto noseIt = m_registeredIntersections.begin(); // pointer to the first SpecificIntersection behind the vehicle's nose
		for (auto it = m_registeredIntersections.begin(); it != m_registeredIntersections.end(); ++it)
		{
			double d = computeDistance(*it->connection, it->intersection->getMyArcPosition(*it->connection));
			if (d < -m_length - it->intersection->getWaitingDistance())
			{
				++noseIt;
				++tailIt;
			}
			else if (d > it->intersection->getWaitingDistance())
			{
				break;
			}
			else
			{
				// All intersections between tailIt and noseIt are now considered as blocked, update them accordingly
				// We use the convention that intersections that are blocked are marked with both remaining distance and arrival time 0.0.
				noseIt->update(0.0, vec2(0.0, currentTime + computeArrivalTime(d + m_length + noseIt->intersection->getWaitingDistance())));
				++noseIt;
			}
		}

		// All registered intersections before can be unregistered
		if (tailIt != m_registeredIntersections.begin())
		{
			m_registeredIntersections.erase(m_registeredIntersections.begin(), tailIt);
		}

		// gather next intersections on my route and updated their registration
		double startPosition = m_currentArcPosition;
		double doneDistance = 0.0;
		double remainingDistance = m_lookaheadDistance;

		for (auto& segment : m_routing.getSegments())
		{
			auto& intersections = segment.connection->getIntersections();
			auto startIt = std::lower_bound(intersections.begin(), intersections.end(), startPosition, [&segment](Intersection* i, double value) {
				return i->getMyArcPosition(*segment.connection) - i->getWaitingDistance() < value;
			});
			auto endIt = std::lower_bound(startIt, intersections.end(), startPosition + remainingDistance, [&segment](Intersection* i, double value) {
				return i->getMyArcPosition(*segment.connection) - i->getWaitingDistance() < value;
			});

			for (/**/; startIt != endIt && noseIt != m_registeredIntersections.end(); ++startIt, ++noseIt)
			{
				if (noseIt != m_registeredIntersections.end())
				{
					if (noseIt->intersection == *startIt)
					{
						double d = (*startIt)->getMyArcPosition(*segment.connection) - startPosition + doneDistance;
						noseIt->update(d, vec2(currentTime + computeArrivalTime(d - (*startIt)->getWaitingDistance()), currentTime + computeArrivalTime(d + m_length + (*startIt)->getWaitingDistance())));
					}
					else
					{
						// This intersection is different from what I expected. Most probably due to a change in the routing.
						// Remove this and all intersections behind. We will register with the new ones in the next loop
						m_registeredIntersections.erase(noseIt, m_registeredIntersections.end());
						break;
					}
				}
			}

			for (/**/; startIt != endIt; ++startIt)
			{
				double d = (*startIt)->getMyArcPosition(*segment.connection) - startPosition + doneDistance;
				m_registeredIntersections.emplace_back(this, *startIt, segment.connection);
				m_registeredIntersections.back().update(d, vec2(currentTime + computeArrivalTime(d - (*startIt)->getWaitingDistance()), currentTime + computeArrivalTime(d + m_length + (*startIt)->getWaitingDistance())));
			}

			remainingDistance -= segment.connection->getCurve().getArcLength() - startPosition;
			doneDistance += segment.connection->getCurve().getArcLength() - startPosition;
			startPosition = 0.0;
			if (remainingDistance <= 0.0)
				break;
		}
	}


	void AbstractVehicle::think()
	{
		m_acceleration = think(m_routing);
	}


	double AbstractVehicle::think(const Routing& routing) const
	{
		if (routing.getSegments().empty())
			return 0.0;

		assert(m_currentConnection == m_routing.getSegments()[0].connection);

		// TODO...
		AccelerationDistance ad = thinkOfVehiclesInFront(m_lookaheadDistance);
		AccelerationDistance ad2 = thinkOfIntersection(ad.considerable ? ad.distance : 0.0);
		//minAcceleration = std::min(minAcceleration, getAcceleration(m_velocity, getEffectiveTargetVelocity(), distanceToIntersection, m_velocity));
		return std::min(ad.acceleration, ad2.acceleration);
	}


	void AbstractVehicle::move(double tickLength)
	{
		if (m_currentConnection == nullptr)
			return;

		m_velocity = std::max(0.0, m_velocity + m_acceleration);
		const double arcLengthToMove = m_velocity * tickLength * 10.0;

		m_currentArcPosition += arcLengthToMove;
		if (m_currentArcPosition > m_currentConnection->getCurve().getArcLength())
		{
			m_currentArcPosition -= m_currentConnection->getCurve().getArcLength();
			if (m_routing.getSegments().size() == 1)
			{
				setCurrentConnection(nullptr);
				return;
			}
			else
			{
				m_visitedConnections.push_back(m_currentConnection);
				setCurrentConnection(m_routing.getSegments()[1].connection);
				updateRouting(m_currentConnection->getStartNode(), m_destinationNodes);
			}
		}
	}


	double AbstractVehicle::computeArrivalTime(double distance) const
	{
		if (distance < 0.0)
			return 0.0;

		// distance is in dm, velocity in m/s. For easier calculations, we transform the distance unit to meters.
		distance /= 10;

		const bool keepVelocity = false;
		if (keepVelocity)
		{
			return distance / m_velocity;
		}
		else
		{
			double alreadyCoveredDistance = 0.0;
			int alreadySpentTime = 0;
			double currentVelocity = m_velocity;
			double effDesVel = getEffectiveTargetVelocity();

			while (alreadyCoveredDistance <= distance)
			{
				currentVelocity += getAcceleration(currentVelocity, effDesVel);
				alreadyCoveredDistance += currentVelocity;
				alreadySpentTime++;
			}

			return alreadySpentTime - ((alreadyCoveredDistance - distance) / currentVelocity);
		}
	}


	AbstractVehicle::AccelerationDistance AbstractVehicle::thinkOfVehiclesInFront(double lookaheadDistance) const
	{
		// Find the next vehicle in front of me
		// TODO: The original code also considers parallel connections if we're currently at the very beginning of our 
		// current connection. However, I would assume that this should also be covered by the intersection handling code.
		VehicleDistance vd = m_currentConnection->getVehicleBehind(m_currentArcPosition, lookaheadDistance);

		if (vd.empty())
		{
			return{ false, getAcceleration(m_velocity, getEffectiveTargetVelocity(), lookaheadDistance, m_velocity), lookaheadDistance };
		}
		else
		{
			const double distance = vd.distance - vd.vehicle->getLength();
			return{ vd.vehicle->m_acceleration < 0.0, getAcceleration(m_velocity, getEffectiveTargetVelocity(), distance, m_velocity - vd.vehicle->m_velocity), distance };
		}
	}


	AbstractVehicle::AccelerationDistance AbstractVehicle::thinkOfIntersection(double stopPoint) const
	{
		const double s0 = getDesiredDistance(0, 0);
		for (auto it = m_registeredIntersections.begin(); it != m_registeredIntersections.end(); ++it)
		{
			auto& si = *it;
			bool waitInFront = false;
			bool avoidBlocking = true;

			auto& myCvt = si.intersection->getCrossingVehicleInfo(*this, *si.connection);
			auto cvtList = si.intersection->computeInterferingVehicles(*this, *si.connection);
			const Connection& otherConnection = si.intersection->getOtherConnection(*si.connection);

			// We do not need to consider already blocked intersections
			// (We really want to leave them as soon as possible, so don't even think of breaking for them ;))
			if (myCvt.remainingDistance <= 0.0)
				continue;

			// If other connection is more important than mine
			if (otherConnection.getPriority() > si.connection->getPriority())
			{
				// If there is any interfering vehicle, I should wait in front of the intersection.
				// TODO:	Develop s.th. more convenient (e.g. if possible, try to accelerate a little to get through first).
				if (!cvtList.empty())
					waitInFront = true;

				// Intersection is close to stop point so that vehicle will block this intersection => wait in front
				if ((stopPoint > myCvt.remainingDistance) && (stopPoint - m_length - s0 < myCvt.remainingDistance) && (si.intersection->avoidBlocking()))
					waitInFront = true;
			}
			// Both connections have the same priority
			else if (otherConnection.getPriority() == si.connection->getPriority())
			{
				// Intersection is close to stop point so that vehicle will block this intersection => wait in front
				if ((stopPoint > myCvt.remainingDistance) && (stopPoint - m_length - s0 < myCvt.remainingDistance) && (si.intersection->avoidBlocking()))
					waitInFront = true;

				// check at each intersection, which vehicle was there first
				for (auto& otherCvt : cvtList)
				{
					// I should wait if:
					//  - The other vehicle originally reached the intersection before me
					//  - TODO: I would block him significantly if I continue.
					if (myCvt.originalArrivalTime > otherCvt.originalArrivalTime || otherCvt.remainingDistance < 0)
					{
						waitInFront = true;
						break;
					}

					// I should also wait if the other vehicle is already blocking the intersection
					if (otherCvt.remainingDistance <= 0.0)
					{
						waitInFront = true;
						avoidBlocking = false;
						break;
					}
				}
			}
			// My connection is more important than the other one
			else
			{
				// If the other vehicle hasn't reached the intersection yet, nothing is to do here.
				// Above is ensured, that the other vehicle will wait (hopefully, it does... o_O)

				// If otherwise the other vehicle is already blocking the intersection, I'm doing good in waiting in front of it.
				for (auto& otherCvt : cvtList)
				{
					if (otherCvt.remainingDistance <= 0)
					{
						waitInFront = true;
						avoidBlocking = false;
						break;
					}
				}
			}

			if (waitInFront)
			{
				if (avoidBlocking)
				{
					// first go backwards: If I wait before an intersection I might block other intersections before.
					// This is usually not wanted, therefore go backwards and wait in front of the first intersection 
					// where I won't block any other.
					double blockedIntersectionStart = myCvt.remainingDistance - si.intersection->getWaitingDistance();
					auto rit = std::make_reverse_iterator(it);
					for (/**/; rit != m_registeredIntersections.rend(); ++rit)
					{
						auto& prevSi = *rit;
						auto& prevCvt = prevSi.intersection->getCrossingVehicleInfo(*this, *prevSi.connection);

						// do not consider intersections that I am already blocking - I can't help this anymore
						if (prevCvt.remainingDistance <= 0.0)
						{
							break;
						}

						double prevIntersectionEnd = prevCvt.remainingDistance + prevSi.intersection->getWaitingDistance();
						// check whether intersection will be blocked
						if (prevCvt.remainingDistance > -(m_length + prevSi.intersection->getWaitingDistance()) && blockedIntersectionStart - prevIntersectionEnd < m_length + s0)
						{
							// intersection will be blocked and both NodeConnections from the intersection do not originate from the same node 
							// => wait in front of it and continue looking backwards
							if (prevSi.intersection->avoidBlocking())
							{
								blockedIntersectionStart = prevIntersectionEnd - 2 * prevSi.intersection->getWaitingDistance();
							}
							// intersection will be blocked but both NodeConnections from the intersection originate from the same LineNode
							else
							{
								// this intersection is allowed to be blocked => nothing to do here
							}
						}
						else
						{
							// will not be blocked => no further search necessary
							break;
						}
					}
					it = rit.base();
				}

				const double distance = it->intersection->getCrossingVehicleInfo(*this, *it->connection).remainingDistance - it->intersection->getWaitingDistance();

				// Update this and all following intersections, that I won't cross in the near future.
				for (/**/; it != m_registeredIntersections.end(); ++it)
				{
					it->setWait(true);
				}

				return{ true, getAcceleration(m_velocity, getEffectiveTargetVelocity(), distance, m_velocity), distance };
			}
			else
			{
				it->setWait(false);
			}
		}


		return{ false, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity() };
	}


	void AbstractVehicle::updateRouting(const Node& startNode, std::vector<Node*> destinationNodes)
	{
		m_routing.compute(startNode, destinationNodes, *this);
	}


	double AbstractVehicle::computeDistance(const Connection& connection, double arcPos) const
	{
		if (&connection == m_currentConnection)
			return arcPos - m_currentArcPosition;

		// check visited nodes
		{
			double acc = -m_currentArcPosition;
			for (auto it = m_visitedConnections.rbegin(); it != m_visitedConnections.rend(); ++it)
			{
				if (&connection == *it)
					return acc - ((*it)->getCurve().getArcLength() - arcPos);
				acc -= (*it)->getCurve().getArcLength();
			}
		}

		// check upcoming nodes
		{
			double acc = m_currentConnection->getCurve().getArcLength() - m_currentArcPosition;
			for (auto& c : m_routing.getSegments())
			{
				if (&connection == c.connection)
					return acc + arcPos;
				acc += c.connection->getCurve().getArcLength();
			}
		}

		return std::numeric_limits<double>::quiet_NaN();
	}


	AbstractVehicle::SpecificIntersection::SpecificIntersection(const AbstractVehicle* vehicle, Intersection* intersection, const Connection* connection)
		: vehicle(vehicle)
		, intersection(intersection)
		, connection(connection)
	{}


	AbstractVehicle::SpecificIntersection::~SpecificIntersection()
	{
		intersection->unregisterVehicle(*vehicle, *connection);
	}


	void AbstractVehicle::SpecificIntersection::update(double remainingDistance, vec2 blockingTime) const
	{
		intersection->registerVehicle(*vehicle, *connection, remainingDistance, blockingTime);
	}


	void AbstractVehicle::SpecificIntersection::setWait(bool willWaitInFront) const
	{
		intersection->updateVehicleWait(*vehicle, *connection, willWaitInFront);
	}


}
}
