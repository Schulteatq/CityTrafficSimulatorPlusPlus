#ifndef CTS_CORE_UTILS_H__
#define CTS_CORE_UTILS_H__

#include <utility>

namespace cts
{
	namespace utils
	{

		/// Convenience function implementing the remove-erase idiom to remove all elements from 
		/// \e container that match \e value.
		template<typename Container, typename T>
		inline void remove_erase(Container& container, T&& value)
		{
			container.erase(std::remove(container.begin(), container.end(), std::forward<T>(value)), container.end());
		}


		/// Convenience function implementing the remove-erase idiom to remove all elements from 
		/// \e container for which \e predicate yields true.
		template<typename Container, typename UnaryPredicate>
		inline void remove_erase_if(Container& container, UnaryPredicate&& predicate)
		{
			container.erase(std::remove_if(container.begin(), container.end(), std::forward<UnaryPredicate>(predicate)), container.end());
		}


	}
}

#endif
