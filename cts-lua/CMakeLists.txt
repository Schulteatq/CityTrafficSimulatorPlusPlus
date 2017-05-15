# Define library. Only source files here!
project(cts-lua LANGUAGES CXX)

file(GLOB_RECURSE Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
)
file(GLOB_RECURSE Headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/*.h
)

add_library(cts-lua ${Sources} ${Headers})
target_link_libraries(cts-lua PUBLIC liblua cts-core)
target_compile_definitions(cts-lua PUBLIC "SOL_CHECK_ARGUMENTS")
target_include_directories(cts-lua
  PRIVATE
    ${CtsGlobalIncludeDirs}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../ext/sol2>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../ext/lua-5.3/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(cts-lua PUBLIC "CTS_LUA_BUILD_SHARED_LIBS")
endif()
set_target_properties (cts-lua PROPERTIES DEFINE_SYMBOL "CTS_LUA_EXPORTS")

DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsCoreSources ${CtsHome}/cts-lua "src")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsCoreHeaders ${CtsHome}/cts-lua "include/cts-core")
