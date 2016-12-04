# Define library. Only source files here!
project(tinyxml2 VERSION 4.0.1 LANGUAGES CXX)

add_library(tinyxml2 "include/tinyxml2.h" "src/tinyxml2.cpp")
add_definitions(${CtsGlobalDefinitions})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")


if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set_target_properties (tinyxml2 PROPERTIES DEFINE_SYMBOL "TINYXML2_EXPORT")

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(tinyxml2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

# 'make install' to the correct location
install(TARGETS tinyxml2 EXPORT TinyXml2Config
    ARCHIVE  DESTINATION lib
    LIBRARY  DESTINATION lib
    RUNTIME  DESTINATION bin)  # This is for Windows
install(DIRECTORY include/ DESTINATION include)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into <prefix>/cmake.
install(EXPORT TinyXml2Config DESTINATION share/tinyxml2/cmake)

# This makes the project importable from the build directory
export(TARGETS tinyxml2 FILE TinyXml2Config.cmake)
