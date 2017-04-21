cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(CityTrafficSimulator++ VERSION 0.1 LANGUAGES CXX)
include("cmake/commonconf.cmake")

if(WIN32)
  # Disable warnings for Microsoft compiler:
  # C4251  class needs to have dll interface (used for std classes)
  # C4503: The decorated name was longer than the compiler limit (4096), and was truncated.
  #        Happens with some Eigen includes.
  # C4068: Unknown Pragma since we use some GCC pragmas in the code.
  LIST(APPEND CtsGlobalDefinitions /wd4251 /wd4503 /wd4068 /W4)

  # enable parallel builds in Visual Studio
  LIST(APPEND CtsGlobalDefinitions "/MP" "-D_CRT_SECURE_NO_DEPRECATE")
endif(WIN32)

LIST(APPEND CtsGlobalIncludeDirs "${CMAKE_CURRENT_SOURCE_DIR}/ext/Eigen")

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Resolve Qt5 DLL locations
MACRO(RESOLVE_QT5_DLL_LOCATIONS Components)
    FOREACH(component ${Components})
        GET_TARGET_PROPERTY(DebugLocation "Qt5::${component}" LOCATION_DEBUG)
        GET_TARGET_PROPERTY(ReleaseLocation "Qt5::${component}" LOCATION)
        LIST(APPEND CampvisExternalDllsDebug ${DebugLocation})
        LIST(APPEND CampvisExternalDllsRelease ${ReleaseLocation})
    ENDFOREACH()
ENDMACRO(RESOLVE_QT5_DLL_LOCATIONS)

option(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)
resolve_qt5_dll_locations("Core;Gui;Widgets")

# External 3rd party libs that we include
add_subdirectory(ext/tinyxml2)

# Targets that we develop
add_subdirectory(cts-core)
add_subdirectory(cts++)
