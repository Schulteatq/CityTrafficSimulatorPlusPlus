#ifndef CTS_CORE_INTERSECTION_H__
#define CTS_CORE_INTERSECTION_H__

#include <cts-core/coreapi.h>

#include <map>
#include <memory>
#include <vector>

namespace cts { namespace core
{
	class Connection;

	/**
	 * Represents a logical intersection between two network connections.
	 * An Intersection is parameterized by the two connections intersecting and the its location
	 * on them.
	 */
	class CTS_CORE_API Intersection
	{
	public:
		/// Structure storing information on a vehicle that is going to cross this intersection
		struct CrossingVehicleInfo
		{
			double originalArrivalTime;	///< Time when the vehicle originally planned to arrive at the intersection.
			double remainingDistance;	///< Remaining distance of the vehicle to the intersection.
			vec2 blockingTime;			///< Simulation time interval when the vehicle is going to block the intersection.
			bool willWaitInFront;		///< Flag whether the vehicle is going to wait in front of the intersection.
		};


		/// Creates a new Intersection with the given parameters.
		/// \param	aConnection     First network connection intersecting.
		/// \param	aTime           Location of the intersection on aConnection.
		/// \param	bConnection     Second network connection intersecting.
		/// \param	bTime           Location of the intersection on bConnection.
		Intersection(const Connection& aConnection, double aTime, const Connection& bConnection, double bTime);

		/// Checks whether vehicles should keep this intersection clear in case that they won't be 
		/// able to pass it completely.
		bool avoidBlocking() const;

		/// Returns the first network connection intersection.
		const Connection& getFirstConnection() const;
		/// Returns the second network connection intersection.
		const Connection& getSecondConnection() const;
		const Connection& getOtherConnection(const Connection& connection) const;

		double getFirstTime() const;
		double getSecondTime() const;
		double getMyTime(const Connection& connection) const;

		double getFirstArcPosition() const;
		double getSecondArcPosition() const;
		double getMyArcPosition(const Connection& connection) const;

		vec2 getFirstCoordinate() const;
		vec2 getSecondCoordinate() const;

		/// Return the distance vehicles should keep in case they need to wait in front.
		double getWaitingDistance() const;


		/// Registers \e vehicle with this intersection or updates the vehicles crossing info if it is already registered.
		/// \param  vehicle				The vehicle that is going to use this intersection
		/// \param  connection			The connection the vehicle is going to use. Must be one of the two connections defining this intersection.
		/// \param  remainingDistance	Remaining distance of the vehicle to the intersection (arc length).
		/// \param  blockingTime		Simulation time interval this vehicle will block the intersection.
		void registerVehicle(const AbstractVehicle& vehicle, const Connection& connection, double remainingDistance, vec2 blockingTime);

		/// Updates the waiting status how the given vehicle will interact with this intersection.
		/// \param  vehicle				The vehicle that is going to use this intersection
		/// \param  connection			The connection the vehicle is going to use. Must be one of the two connections defining this intersection.
		/// \param  willWaitInFront		Flag whether the vehicle is going to wait in front of the intersection.
		void updateVehicleWait(const AbstractVehicle& vehicle, const Connection& connection, bool willWaitInFront);

		/// Unregisters \e vehicle with this intersection.
		/// \param  vehicle				The vehicle to unregister. Must be registered.
		/// \param  connection			The connection the vehicle used/planned to use. Must be one of the two connections defining this intersection.
		void unregisterVehicle(const AbstractVehicle& vehicle, const Connection& connection);

		/// Returns the list of all crossing entities that interfere with the given vehicle
		/// \param  vehicle				The vehicle that is going to use this intersection, must be registered with this intersection.
		/// \param  connection			The connection the vehicle is going to use. Must be one of the two connections defining this intersection.
		std::vector<CrossingVehicleInfo> computeInterferingVehicles(const AbstractVehicle& vehicle, const Connection& connection);

		CrossingVehicleInfo& getCrossingVehicleInfo(const AbstractVehicle& vehicle, const Connection& connection);

	private:
		const Connection* m_aConnection;    ///< First network connection intersecting.
		const Connection* m_bConnection;    ///< Second network connection intersecting.

		double m_aTime;                     ///< Location of the intersection on aConnection in terms of parameterization time.
		double m_bTime;                     ///< Location of the intersection on bConnection in terms of parameterization time.
		double m_aArcPosition;              ///< Location of the intersection on aConnection in terms of arc length.
		double m_bArcPosition;              ///< Location of the intersection on bConnection in terms of arc length.
		double m_waitingDistance;           ///< Distance vehicles should keep in case they need to wait in front.

	public:
		/// List of all vehicles registered with the first network connection.
		std::map<const AbstractVehicle*, CrossingVehicleInfo> m_aCrossingVehicles;
		/// List of all vehicles registered with the second network connection.
		std::map<const AbstractVehicle*, CrossingVehicleInfo> m_bCrossingVehicles;
	};
}
}

#endif
