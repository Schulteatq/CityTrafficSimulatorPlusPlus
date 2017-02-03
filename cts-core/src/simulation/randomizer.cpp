#include <cts-core/simulation/randomizer.h>

#include <cassert>
#include <limits>
#include <random>

namespace cts { namespace core
{
	struct Randomizer::Impl
	{
		std::minstd_rand engine;
	};


	Randomizer::Randomizer()
		: m_pimpl(new Impl())
	{
		reset(42);
	}


	Randomizer::~Randomizer()
	{
	}


	void Randomizer::reset(uint32_t seed)
	{
		m_pimpl->engine = std::minstd_rand(seed);
	}


	uint32_t Randomizer::nextInt(uint32_t modulus) const
	{
		assert(modulus <= m_pimpl->engine.max());
		return m_pimpl->engine() % modulus;
	}


	double Randomizer::nextDouble() const
	{
		return double(m_pimpl->engine()) / double(m_pimpl->engine.max());
	}
	

}
}
