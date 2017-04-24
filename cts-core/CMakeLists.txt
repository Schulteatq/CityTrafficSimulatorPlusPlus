# Define library. Only source files here!
project(cts-core VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE CtsCoreSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
)

file(GLOB_RECURSE CtsCoreHeaders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/cts-core/*.h
)

add_library(cts-core ${CtsCoreSources} ${CtsCoreHeaders})

if(WIN32)
  target_compile_definitions(cts-core PUBLIC "-D_CRT_SECURE_NO_DEPRECATE")
endif()
set_target_properties(cts-core PROPERTIES DEFINE_SYMBOL "CTS_CORE_EXPORTS")
if(BUILD_SHARED_LIBS)
  target_compile_definitions(cts-core PRIVATE "-DCTS_CORE_BUILD_SHARED_LIBS")
endif()
if(ENABLE_SCRIPTING)
  target_compile_definitions(cts-core PUBLIC "CTS_ENABLE_SCRIPTING")
endif()

target_include_directories(cts-core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../ext/Eigen>
    $<INSTALL_INTERFACE:include/ext>
)

target_compile_features(cts-core
    PUBLIC cxx_auto_type
    PRIVATE cxx_variadic_templates)

# Depend on a library that we defined in the top-level file
target_link_libraries(cts-core PRIVATE tinyxml2)


DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsCoreSources ${CtsHome}/cts-core "src")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsCoreHeaders ${CtsHome}/cts-core "include/cts-core")
