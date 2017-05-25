#ifndef CTS_LUA_CONFIG_H__
#define CTS_LUA_CONFIG_H__

#ifdef CTS_LUA_BUILD_SHARED_LIBS
#ifdef CTS_LUA_EXPORTS
	// building library -> export symbols
	#ifdef WIN32
		#define CTS_LUA_API __declspec(dllexport)
	#else
		#define CTS_LUA_API
	#endif
#else
	// including library -> import symbols
	#ifdef WIN32
		#define CTS_LUA_API __declspec(dllimport)
	#else
		#define CTS_LUA_API
	#endif
#endif
#else
	#define CTS_LUA_API
#endif

#endif
