#ifndef CTS_CORE_SIMULATION_H__
#define CTS_CORE_SIMULATION_H__

#include <cts-core/coreapi.h>

#include <memory>

namespace std
{
	class thread;
}

namespace cts { namespace core
{
	class Network;
	class Randomizer;

	class CTS_CORE_API Simulation
	{
	public:
		Simulation(Network& network);
		~Simulation();

		/// Returns the simulation speed.
		double getSpeed() const;
		/// Sets the simulation speed, value must be >= 1.0.
		void setSpeed(double value);

		/// Returns the number of simulation steps per simulated second.
		double getTicksPerSecond() const;
		/// Sets the number of simulation steps per simulated second.
		void setTicksPerSecond(double value);

		/// Resets the entire simulation
		void reset(uint32_t randomSeed);

		/// Performs a single simulation step.
		void step();

		/// Starts the simulation for the given duration.
		void start(double duration);

		/// Stops the simulation if it's currently running.
		void stop();

	private:
		void simulationLoop();

		double m_speed;
		double m_ticksPerSecond;
		double m_duration;

		double m_currentTime;
		bool m_stopSimulation;

		std::unique_ptr<std::thread> m_simulationThread;
		std::unique_ptr<Randomizer> m_randomizer;
		Network& m_network;
	};


}
}

#endif // CTS_NETWORK_H__
