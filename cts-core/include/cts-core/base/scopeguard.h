#ifndef CTS_CORE_SCOPEGUARD_H__
#define CTS_CORE_SCOPEGUARD_H__

#include <utility>

namespace cts
{
	namespace utils
	{
		/**
		* Templated proxy class to implement generic scope guards.
		* ScopeGuard ensures that the function given during initialization is executed when the ScopeGuard
		* instance is destroyed (i.e. leaves its scope) unless the guard is dismissed before.
		*
		* \note		Implementation inspired by Andrei Alexandrescu's talk on Systematic Error Handling in C++.
		* \see		makeScopeGuard()
		*/
		template <class ReleaseFunc>
		class ScopeGuard {
		public:
			ScopeGuard(ReleaseFunc&& f)
				: m_func(std::forward<ReleaseFunc>(f))
				, m_active(true)
			{}
			ScopeGuard() = delete;
			~ScopeGuard() { if (m_active) m_func(); }
			ScopeGuard(const ScopeGuard&) = delete;
			ScopeGuard& operator=(const ScopeGuard&) = delete;
			ScopeGuard(ScopeGuard&& rhs)
				: m_func(std::move(rhs.m_func))
				, m_active(rhs.m_active)
			{
				rhs.dismiss();
			}

			void dismiss() { m_active = false; }

		private:
			ReleaseFunc m_func;
			bool m_active;
		};


		/// Generator function to instantiate a ScopeGuard without writing out the templates.
		/// \note	Make sure to capture the returned object.
		template<class ReleaseFunc>
		ScopeGuard<ReleaseFunc> makeScopeGuard(ReleaseFunc&& f) {
			return ScopeGuard<ReleaseFunc>(std::forward<ReleaseFunc>(f));
		}


		/// Enhanced Generator function to first execute the given AcquisitionFunc and then instantiate a ScopeGuard with the given ReleaseFunc.
		/// Some people prefer this overload since it has the code for acquisition and release at the same place right next to each other.
		/// \note	Make sure to capture the returned object.
		template<class AcquisitionFunc, class ReleaseFunc>
		ScopeGuard<ReleaseFunc> makeScopeGuard(AcquisitionFunc&& acquisitionFunc, ReleaseFunc&& releaseFunc) {
			acquisitionFunc();
			return ScopeGuard<ReleaseFunc>(std::forward<ReleaseFunc>(releaseFunc));
		}
	}
}

#endif
