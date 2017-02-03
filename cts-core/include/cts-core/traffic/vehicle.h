#ifndef CTS_CORE_VEHICLE_H__
#define CTS_CORE_VEHICLE_H__

#include <cts-core/coreapi.h>
#include <cts-core/network/routing.h>

#include <vector>

namespace cts { namespace core
{
	class Connection;
	class Node;

	/**
	 * Abstract base class for all vehicles that move through the network.
	 */
	class CTS_CORE_API AbstractVehicle
	{
	public:
		AbstractVehicle(const Node& start, const std::vector<Node*> destination, double targetVelocity);
		virtual ~AbstractVehicle() = default;



		double getTargetVelocity() const;
		double getEffectiveTargetVelocity() const;

		const Connection* getCurrentConnection() const;
		void setCurrentConnection(const Connection* value);

		double getCurrentArcPosition() const;
		void setCurrentArcPosition(double value);

		double getLength() const;



		void think();
		double think(const Routing& routing, double arcPos) const;

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

	protected:
		double thinkOfVehiclesInFront(const Routing& routing, double arcPos) const;
	

		Routing m_routing;
		double m_targetVelocity;
		double m_multiplierTargetVelocity;
		double m_acceleration;
		double m_velocity;

		const Connection* m_currentConnection;
		std::vector<Node*> m_destinationNodes;
		double m_currentArcPosition;
		double m_length;
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
