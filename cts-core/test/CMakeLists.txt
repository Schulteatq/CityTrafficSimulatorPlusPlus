# Define library. Only source files here!
project(cts-core-test LANGUAGES CXX)

file(GLOB_RECURSE Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.cpp
)

add_executable(cts-core-test ${Sources})
target_include_directories(cts-core-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "../../ext/catch")
target_link_libraries(cts-core-test PRIVATE cts-core)
