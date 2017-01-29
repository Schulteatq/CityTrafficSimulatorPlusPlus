#ifndef CTS_CORE_RANDOMIZER_H__
#define CTS_CORE_RANDOMIZER_H__

#include <cts-core/coreapi.h>
#include <memory>

namespace cts { namespace core
{
	/**
	 * Central, global randomizer singleton instance.
	 * 
	 * Allows for reproducing random experiments based on the seed number.
	 */
	class CTS_CORE_API Randomizer
	{
	public:
		/// Default constructor
		Randomizer();
		/// Default destructor
		~Randomizer();


		/// Resets the random number generator to the given seed.
		/// \param  seed	Seed value to use for the randomizer.
		void reset(uint32_t seed);

		/// Generates a new integer random value in the range [0, modulus).
		/// \param  modulus		Modulus for the generated random value.
		uint32_t nextInt(uint32_t modulus);
		
		/// Generates a new double random value in the range [0, 1].
		double nextDouble();

	private:
		// we don't want to expose the expensive std::random stuff here.
		struct Impl;
		std::unique_ptr<Impl> m_pimpl;
	};
}
}

#endif
