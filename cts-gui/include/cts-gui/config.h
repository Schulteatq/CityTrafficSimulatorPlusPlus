#ifndef CTS_GUI_CONFIG_H__
#define CTS_GUI_CONFIG_H__

#ifdef CTS_GUI_BUILD_SHARED_LIBS
#ifdef CTS_GUI_EXPORTS
	// building library -> export symbols
	#ifdef WIN32
		#define CTS_GUI_API __declspec(dllexport)
	#else
		#define CTS_GUI_API
	#endif
#else
	// including library -> import symbols
	#ifdef WIN32
		#define CTS_GUI_API __declspec(dllimport)
	#else
		#define CTS_GUI_API
	#endif
#endif
#else
	#define CTS_GUI_API
#endif

#endif
