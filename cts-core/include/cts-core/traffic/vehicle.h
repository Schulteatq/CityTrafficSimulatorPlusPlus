#ifndef CTS_CORE_VEHICLE_H__
#define CTS_CORE_VEHICLE_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/utils.h>
#include <cts-core/network/routing.h>

#include <deque>
#include <vector>

namespace cts { namespace core
{
	class Connection;
	class Node;

	/**
	 * Abstract base class for all vehicles that move through the network.
	 */
	class CTS_CORE_API AbstractVehicle : public utils::NotCopyable
	{
	public:
		int debugId;

		AbstractVehicle(const Node& start, const std::vector<Node*> destination, double targetVelocity);
		virtual ~AbstractVehicle() = default;



		/// Returns the target velocity of this vehicle if it was free from any outer constraints.
		double getTargetVelocity() const;
		/// Returns the effective target velocity considering the target velocity multiplier and the 
		/// maximum velocity of the vehicle's current connection. 
		double getEffectiveTargetVelocity() const;

		const Connection* getCurrentConnection() const;
		void setCurrentConnection(const Connection* value);

		double getCurrentArcPosition() const;
		void setCurrentArcPosition(double value);

		double getLength() const;


		void prepare(double currentTime);

		void think();
		double think(const Routing& routing) const;

		void move(double tickLength);


		/// Calculates the desired distance with respect to the given parameters.
		/// \param	velocity    Current velocity.
		/// \param	vDiff       Velocity difference to vehicle in front.
		virtual double getDesiredDistance(double velocity, double vDiff) const = 0;

		/// Calculates the acceleration based on the given parameters in case that there is no vehicle in front.
		/// \param	velocity            Current velocity.
		/// \param	desiredVelocity     Desired velocity.
		virtual double getAcceleration(double velocity, double desiredVelocity) const = 0;

		/// Calculates the acceleration based on the given parameters in case that there is a vehicle in front.
		/// \param	velocity            Current velocity.
		/// \param	desiredVelocity     Desired velocity.
		/// \param	distance            Distance to vehicle in front.
		/// \param	vDiff               Velocity difference to vehicle in front.
		virtual double getAcceleration(double velocity, double desiredVelocity, double distance, double vDiff) const = 0;


		double computeArrivalTime(double distance) const;

	protected:
		/// Structure encapsulating a registered intersection.
		/// Takes care of registering/unregistering.
		struct CTS_CORE_API SpecificIntersection : public utils::NotCopyable
		{
			SpecificIntersection(const AbstractVehicle* vehicle, Intersection* intersection, const Connection* connection);
			~SpecificIntersection();

			void update(double remainingDistance, vec2 blockingTime) const;
			void setWait(bool willWaitInFront) const;

			const AbstractVehicle* vehicle;
			Intersection* intersection;
			const Connection* connection;
		};

		struct AccelerationDistance
		{
			bool considerable; //< FIXME: find a better name
			double acceleration;
			double distance;
		};

		AccelerationDistance thinkOfVehiclesInFront(double lookaheadDistance) const;

		AccelerationDistance thinkOfIntersection(double lookaheadDistance) const;

		/// Computes the new routing for this vehicle and updates all internal (e.g. registered intersections) data accordingly.
		/// \param  startNode			Start node
		/// \param  destinationNodes	Destination nodes
		void updateRouting(const Node& startNode, std::vector<Node*> destinationNodes);

		double computeDistance(const Connection& connection, double arcPos) const;
	
		static const double m_lookaheadDistance;

		double m_targetVelocity;
		double m_multiplierTargetVelocity;
		double m_acceleration;
		double m_velocity;

		const Connection* m_currentConnection;
		std::vector<Node*> m_destinationNodes;
		double m_currentArcPosition;
		double m_length;

	private:
		Routing m_routing;						///< Route that the vehicle is planning to use, includes current connection
		std::list<SpecificIntersection> m_registeredIntersections;
		std::vector<const Connection*> m_visitedConnections;

	};


	// ================================================================================================


	class CTS_CORE_API IdmMobil
	{
	public:
		IdmMobil();

		double getDesiredDistance(double velocity, double vDiff) const;

		double getAcceleration(double velocity, double desiredVelocity) const;

		double getAcceleration(double velocity, double desiredVelocity, double distance, double vDiff) const;


	protected:
		double m_safetyDistanceTime;
		double m_maximumAcceleration;
		double m_comfortDeceleration;
		double m_minimumDistance;
		double m_maximumDeceleration;

		double m_politeness;
		double m_lineChangingThreshold;
	};


	template<typename DrivingModelT>
	class TypedVehicle final : public AbstractVehicle
	{
	public:
		using DrivingModel = DrivingModelT;

		TypedVehicle(const Node& start, const std::vector<Node*> destination, double targetVelocity);
		virtual ~TypedVehicle() = default;


		virtual double getDesiredDistance(double velocity, double vDiff) const override
		{
			return m_drivingModel.getDesiredDistance(velocity, vDiff);
		}


		virtual double getAcceleration(double velocity, double desiredVelocity) const override
		{
			return m_drivingModel.getAcceleration(velocity, desiredVelocity);
		}


		virtual double getAcceleration(double velocity, double desiredVelocity, double distance, double vDiff) const override
		{
			return m_drivingModel.getAcceleration(velocity, desiredVelocity, distance, vDiff);
		}

	protected:
		DrivingModel m_drivingModel;
	};


	// ================================================================================================


	template<typename DrivingModelT>
	TypedVehicle<DrivingModelT>::TypedVehicle(const Node& start, const std::vector<Node*> destination, double targetVelocity)
		: AbstractVehicle(start, destination, targetVelocity)
	{

	}


}
}

#endif
