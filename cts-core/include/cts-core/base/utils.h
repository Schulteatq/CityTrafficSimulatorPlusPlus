#ifndef CTS_CORE_UTILS_H__
#define CTS_CORE_UTILS_H__

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

namespace cts
{
	namespace utils
	{
		/// Utility base class for non-copyable but movable classes.
		struct NotCopyable
		{
			NotCopyable() = default;
			NotCopyable(const NotCopyable&) = delete;
			NotCopyable& operator=(const NotCopyable&) = delete;
			NotCopyable(NotCopyable&&) = default;
			NotCopyable& operator=(NotCopyable&&) = default;
		};


		/// Convenience function for checking whether a container contains an element with the given value.
		template<typename Container, typename T>
		inline bool contains(const Container& container, T&& value)
		{
			return std::find(container.begin(), container.end(), std::forward<T>(value)) != container.end();
		}
		

		/// Convenience function implementing the remove-erase idiom to remove all elements from 
		/// \e container that match \e value.
		template<typename Container, typename T>
		inline void remove_erase(Container& container, T&& value)
		{
			container.erase(std::remove(container.begin(), container.end(), std::forward<T>(value)), container.end());
		}
		
		/// Convenience function implementing the remove-erase idiom to remove all elements from 
		/// \e container containing unique_ptrs that match \e value.
		template<typename Container, typename T>
		inline void remove_erase_unique_ptr(Container& container, T* value)
		{
			container.erase(std::remove_if(container.begin(), container.end(), [value](const std::unique_ptr<T>& ptr) { return ptr.get() == value; }), container.end());
		}
		
		/// Convenience function implementing the remove-erase idiom to remove all elements from 
		/// \e container for which \e predicate yields true.
		template<typename Container, typename UnaryPredicate>
		inline void remove_erase_if(Container& container, UnaryPredicate&& predicate)
		{
			container.erase(std::remove_if(container.begin(), container.end(), std::forward<UnaryPredicate>(predicate)), container.end());
		}


		/// Convenience function giving std::accumulate a more intuitive name and taking an entire 
		/// container instead of iterators.
		template<typename Container, typename T, typename BinaryOperation>
		inline T reduce(const Container& container, T init, BinaryOperation&& op)
		{
			return std::accumulate(container.begin(), container.end(), init, op);
		}


		/// Converts a vector of unique_ptrs to a vector of reference_wrappers.
		/// Inspired by https://jonasdevlieghere.com/containers-of-unique-pointers/
		template<typename T>
		std::vector< std::reference_wrapper<T> > to_refs(const std::vector< std::unique_ptr<T> >& container)
		{
			std::vector< std::reference_wrapper<T> > refs;
			refs.reserve(container.size());
			std::transform(container.begin(), container.end(), std::back_inserter(refs),
				[](auto &x) { return std::ref(*x); });
			return refs;
		}
	}
}

#endif
