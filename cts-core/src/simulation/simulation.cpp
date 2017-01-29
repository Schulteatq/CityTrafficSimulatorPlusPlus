#include <cts-core/base/randomizer.h>
#include <cts-core/network/network.h>
#include <cts-core/simulation/simulation.h>

#include <cassert>
#include <chrono>
#include <thread>

namespace cts { namespace core
{


	Simulation::Simulation(Network& network)
		: m_speed(1.0)
		, m_ticksPerSecond(15.0)
		, m_duration(0.0)
		, m_currentTime(0.0)
		, m_stopSimulation(false)
		, m_randomizer(new Randomizer())
		, m_network(network)
	{

	}


	Simulation::~Simulation()
	{
		stop();
	}


	double Simulation::getSpeed() const
	{
		return m_speed;
	}


	void Simulation::setSpeed(double value)
	{
		assert(value >= 1.0);
		if (value >= 1.0)
			m_speed = value;
	}


	double Simulation::getTicksPerSecond() const
	{
		return m_ticksPerSecond;
	}


	void Simulation::setTicksPerSecond(double value)
	{
		m_ticksPerSecond = value;
	}


	void Simulation::reset(uint32_t randomSeed)
	{
		m_randomizer->reset(randomSeed);
	}


	void Simulation::step()
	{
		m_network.getTrafficManager().tick(1.0 / m_ticksPerSecond, *m_randomizer);
	}


	void Simulation::start(double duration)
	{
		assert(m_simulationThread == nullptr);
		if (m_simulationThread != nullptr)
			return;

		m_duration = duration;
		m_stopSimulation = false;
		m_simulationThread = std::make_unique<std::thread>([this](){ simulationLoop(); });
	}


	void Simulation::stop()
	{
		if (m_simulationThread)
		{
			m_stopSimulation = true;
			m_simulationThread->join();
		}
	}


	void Simulation::simulationLoop()
	{
		using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
		using TimeRep = int64_t;
		while (m_currentTime < m_duration && !m_stopSimulation)
		{
			const TimePoint timeBefore = std::chrono::system_clock::now();
			step();

			const TimeRep microSecsBetweenSteps = static_cast<TimeRep>(1000000.0 / (m_ticksPerSecond * m_speed));
			const TimePoint timeAfter = std::chrono::system_clock::now();
			const TimeRep timePassed = std::chrono::duration_cast<std::chrono::microseconds>(timeAfter - timeBefore).count();
			if (timePassed < microSecsBetweenSteps)
				std::this_thread::sleep_for(std::chrono::microseconds(microSecsBetweenSteps - timePassed));
		}
	}


}
}
