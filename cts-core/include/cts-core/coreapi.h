#ifndef CTS_CORE_API_H__
#define CTS_CORE_API_H__

#ifdef CTS_CORE_BUILD_SHARED_LIBS
#ifdef CTS_CORE_EXPORTS
	// building library -> export symbols
	#ifdef WIN32
		#define CTS_CORE_API __declspec(dllexport)
	#else
		#define CTS_CORE_API
	#endif
#else
	// including library -> import symbols
	#ifdef WIN32
		#define CTS_CORE_API __declspec(dllimport)
	#else
		#define CTS_CORE_API
	#endif
#endif
#else
	#define CTS_CORE_API
#endif

#endif
