#ifndef CTS_CORE_TRAFFICMANAGER_H__
#define CTS_CORE_TRAFFICMANAGER_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/signal.h>
#include <cts-core/base/utils.h>
#include <cts-core/network/location.h>

#include <memory>
#include <vector>

namespace cts { namespace core
{
	class AbstractVehicle;
	class Simulation;

	/**
	 * Manager class for the traffic of the network. 
	 * 
	 * TrafficManager takes care of spawning vehicles according to the configured traffic density.
	 */
	class CTS_CORE_API TrafficManager : public utils::NotCopyable
	{
	public:
		/// Structure describing the traffic volume from a given location toward a given destination.
		struct TrafficVolume : public utils::NotCopyable
		{
			TrafficVolume(const std::vector<Node*>& start, const std::vector<Node*>& destination);

			Location start;			///< Start nodes where vehicles are supposed to spawn.
			Location destination;	///< Destination nodes of the spawned vehicles.
			int carsPerHour;		///< Traffic density for cars.

			// FIXME: do not have a fixed set of vehicle classes but use some cool tag system
			int trucksPerHour;		///< Traffic density for trucks.
			int busesPerHour;		///< Traffic density for buses.
			int tramsPerHour;		///< Traffic density for trams.
		};


		TrafficManager();
		~TrafficManager();


		TrafficVolume* addVolume(const std::vector<Node*>& start, const std::vector<Node*>& destination);
		void removeVolume(TrafficVolume* volume);
		const std::vector< std::unique_ptr<TrafficVolume> >& getVolumes() const;


		const std::vector< std::unique_ptr<AbstractVehicle> >& getVehicles() const;

		void tick(const Simulation& simulation, double tickLength);

	public:
		Signal<AbstractVehicle*> s_vehicleSpawned;

	private:
		void spawnVehicles(const Simulation& simulation, double tickLength);
		void tickVehicles(const Simulation& simulation, double tickLength);


		std::vector< std::unique_ptr<TrafficVolume> > m_volumes;
		std::vector< std::unique_ptr<AbstractVehicle> > m_vehicles;
		std::vector< TrafficVolume* > m_vehiclesToSpawn;

		double m_globalTrafficMultiplier;

	};

}
}

#endif
