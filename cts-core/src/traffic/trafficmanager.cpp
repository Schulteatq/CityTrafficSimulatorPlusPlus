#include <cts-core/base/randomizer.h>
#include <cts-core/network/connection.h>
#include <cts-core/network/node.h>
#include <cts-core/network/routing.h>
#include <cts-core/traffic/trafficmanager.h>
#include <cts-core/traffic/vehicle.h>

#include <algorithm>
#include <cmath>

namespace cts { namespace core
{


	TrafficManager::TrafficVolume::TrafficVolume(const std::vector<Node*>& start, const std::vector<Node*>& destination)
		: start(start, "")
		, destination(destination, "")
		, carsPerHour(0)
		, trucksPerHour(0)
		, busesPerHour(0)
		, tramsPerHour(0)
	{

	}


	// ================================================================================================


	TrafficManager::TrafficManager()
		: m_globalTrafficMultiplier(1.0)
	{}


	TrafficManager::~TrafficManager() {}


	TrafficManager::TrafficVolume* TrafficManager::addVolume(const std::vector<Node*>& start, const std::vector<Node*>& destination)
	{
		m_volumes.push_back(std::make_unique<TrafficVolume>(start, destination));
		return m_volumes.back().get();
	}


	void TrafficManager::removeVolume(TrafficVolume* volume)
	{
		utils::remove_erase_unique_ptr(m_volumes, volume);
	}


	const std::vector< std::unique_ptr<TrafficManager::TrafficVolume> >& TrafficManager::getVolumes() const
	{
		return m_volumes;
	}


	const std::vector< std::unique_ptr<cts::core::AbstractVehicle> >& TrafficManager::getVehicles() const
	{
		return m_vehicles;
	}


	void TrafficManager::tick(double tickLength, Randomizer& randomizer)
	{
		spawnVehicles(tickLength, randomizer);
		tickVehicles(tickLength, randomizer);
	}


	void TrafficManager::spawnVehicles(double tickLength, Randomizer& randomizer)
	{
		const double time = tickLength * m_globalTrafficMultiplier;
		if (time <= 0.0)
			return;

		for (auto& volume : m_volumes)
		{
			if (volume->start.getNodes().empty() || volume->destination.getNodes().empty())
				continue;

			const uint32_t randomCar = randomizer.nextInt(int(ceil(3600.0 / (time * volume->carsPerHour))));
			if (randomCar == 0)
			{
				// Since the place where the vehicle should spawn might be occupied at this very moment, 
				// spawning vehicles is a two-step process: Here, we just add the TrafficVolume to the list 
				// of vehicles-to-spawn. Below, we then try to spawn all vehicles and only if the spawning 
				// was successful, we remove it from m_vehiclesToSpawn.
				m_vehiclesToSpawn.emplace_back(volume.get());
			}
		}

		for (auto& volume : m_vehiclesToSpawn)
		{
			const uint32_t startIndex = randomizer.nextInt(uint32_t(volume->start.getNodes().size()));
			const Node* start = volume->start.getNodes()[startIndex];

			// make sure that there is sufficient space at this location.
			bool canSpawn = true;
			for (auto& connection : start->getOutgoingConnections())
			{
				if (!connection->getVehicles().empty())
				{
					const AbstractVehicle* v = connection->getVehicles().front();
					if (v->getCurrentArcPosition() < v->getLength() + 20.0) // FIXME: ugly constant hack
					{
						canSpawn = false;
						break;
					}
				}
			}

			if (canSpawn)
			{
				m_vehicles.push_back(std::make_unique< TypedVehicle<IdmMobil> >(*start, volume->destination.getNodes(), 42));
				s_vehicleSpawned.emitSignal(m_vehicles.back().get());
				volume = nullptr;
			}
		}

		// remove all vehicles that were spawned successfully from the list.
		utils::remove_erase(m_vehiclesToSpawn, nullptr);
	}
	

	void TrafficManager::tickVehicles(double tickLength, Randomizer& randomizer)
	{
		for (auto& vehicle : m_vehicles)
		{
			
		}
	}


}
}
