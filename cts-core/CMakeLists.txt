# Define library. Only source files here!
project(cts-core VERSION 0.1 LANGUAGES CXX)

file(GLOB CtsCoreSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    src/*.cpp
    src/base/*.cpp
    src/network/*.cpp
    src/simulation/*.cpp
    src/traffic/*.cpp
)

file(GLOB CtsCoreHeaders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    include/cts-core/*.h
    include/cts-core/base/*.h
    include/cts-core/network/*.h
    include/cts-core/simulation/*.h
    include/cts-core/traffic/*.h
)

add_library(cts-core ${CtsCoreSources} ${CtsCoreHeaders})
add_definitions(${CtsGlobalDefinitions})
include_directories(${CtsGlobalIncludeDirs})

if(BUILD_SHARED_LIBS)
  add_definitions("-DCTS_CORE_BUILD_SHARED_LIBS")
endif()
set_target_properties (cts-core PROPERTIES DEFINE_SYMBOL "CTS_CORE_EXPORTS")

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(cts-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

# If we have compiler requirements for this library, list them
# here
target_compile_features(cts-core
    PUBLIC cxx_auto_type
    PRIVATE cxx_variadic_templates)

# Depend on a library that we defined in the top-level file
target_link_libraries(cts-core tinyxml2)


DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsCoreSources ${CtsHome}/cts-core "src")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsCoreHeaders ${CtsHome}/cts-core "include/cts-core")


# 'make install' to the correct location
install(TARGETS cts-core EXPORT CtsCoreConfig
    ARCHIVE  DESTINATION lib
    LIBRARY  DESTINATION lib
    RUNTIME  DESTINATION bin)  # This is for Windows
install(DIRECTORY include/ DESTINATION include)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into <prefix>/cmake.
install(EXPORT CtsCoreConfig DESTINATION share/CtsCore/cmake)

# This makes the project importable from the build directory
export(TARGETS cts-core FILE CtsCoreConfig.cmake)

# Every library has unit tests, of course
#add_executable(testlib
#    test/testlib.cpp)

#target_link_libraries(testlib
#    lib)

#add_test(testlib testlib)
