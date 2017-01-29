#ifndef CTS_CORE_TICKABLE_H__
#define CTS_CORE_TICKABLE_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/randomizer.h>

namespace cts
{
	namespace core
	{
		class CTS_CORE_API Tickable
		{
		public:
			virtual ~Tickable() = default;

			/// Notifies the class that time has passed for the given amount of time.
			/// \param  tickLength	Amount of time that has passed in terms of seconds.
			/// \param  randomizer	Randomizer to use to generate random numbers.
			virtual void tick(double tickLength, Randomizer& randomizer) = 0;
		};
	}
}

#endif
