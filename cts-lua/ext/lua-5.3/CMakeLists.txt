# Copyright (C) 2007-2015 LuaDist.
# Created by Peter Drahoš, Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( lua C )
cmake_minimum_required ( VERSION 2.8 )

## CONFIGURATION

if ( WIN32 AND NOT CYGWIN )
  # Windows systems
  option ( LUA_USE_WINDOWS "Windows specific build." ON )
  option ( LUA_BUILD_AS_DLL "Build Lua library as Dll." ${BUILD_SHARED_LIBS} )
else ( )
  # Posix systems (incl. Cygwin)
  option ( LUA_USE_POSIX "Use POSIX features." ON )
  option ( LUA_USE_DLOPEN "Use dynamic linker to load modules." ON )
  # Apple and Linux specific
  if ( LINUX OR APPLE )
    option ( LUA_USE_AFORMAT "Assume 'printf' handles 'aA' specifiers" ON )
  endif ( )
endif ( )

## SETUP
# Setup needed variables and libraries
if ( LUA_USE_POSIX )
  # On POSIX Lua links to standard math library "m"
  list ( APPEND LIBS m )
endif ( )

if ( LUA_USE_DLOPEN )
  # Link to dynamic linker library "dl"
	find_library ( DL_LIBRARY NAMES dl )
	if ( DL_LIBRARY )
    list ( APPEND LIBS ${DL_LIBRARY} )
	endif ( )
endif ( )

## SOURCES
# Generate luaconf.h
configure_file ( src/luaconf.h.in ${CMAKE_CURRENT_BINARY_DIR}/cfg/luaconf.h )

# Sources and headers
include_directories ( src ${CMAKE_CURRENT_BINARY_DIR} )
set ( SRC_CORE src/lapi.c src/lcode.c src/lctype.c src/ldebug.c src/ldo.c src/ldump.c
  src/lfunc.c src/lgc.c src/llex.c src/lmem.c src/lobject.c src/lopcodes.c src/lparser.c
  src/lstate.c src/lstring.c src/ltable.c src/ltm.c src/lundump.c src/lvm.c src/lzio.c )
set ( SRC_LIB src/lauxlib.c src/lbaselib.c src/lbitlib.c src/lcorolib.c src/ldblib.c
  src/liolib.c src/lmathlib.c src/loslib.c src/lstrlib.c src/ltablib.c src/linit.c
  src/lutf8lib.c )
set ( SRC_LUA src/lua.c )
set ( SRC_LUAC src/luac.c )

if ( LUA_USE_RELATIVE_LOADLIB )
  # Use modified loadlib
  list ( APPEND SRC_LIB src/loadlib_rel.c )
else ( )
  list ( APPEND SRC_LIB src/loadlib.c )
endif ( )

## BUILD
# Create lua library
add_library(liblua ${SRC_CORE} ${SRC_LIB} ${LUA_DLL_RC})
target_include_directories(liblua PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/cfg)
target_link_libraries(liblua PRIVATE ${LIBS})
set_target_properties(liblua PROPERTIES OUTPUT_NAME lua CLEAN_DIRECT_OUTPUT 1)
if ( LUA_BUILD_AS_DLL )
  target_compile_definitions(liblua PRIVATE LUA_BUILD_AS_DLL)
endif ()

set_target_properties(liblua PROPERTIES FOLDER "Externals")
