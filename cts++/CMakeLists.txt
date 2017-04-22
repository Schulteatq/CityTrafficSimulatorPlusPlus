set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE CtsApplicationSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
)

file(GLOB_RECURSE CtsApplicationHeaders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.h
)


# Define an executable
add_executable(CityTrafficSimulator++ ${CtsApplicationHeaders} ${CtsApplicationSources})
target_include_directories(CityTrafficSimulator++
  PRIVATE src
)

# Define the libraries this project depends upon
target_link_libraries(CityTrafficSimulator++ PRIVATE cts-core cts-gui Qt5::Widgets)

if(ENABLE_SCRIPTING)
  target_link_libraries(CityTrafficSimulator++ PRIVATE cts-lua)
endif()

DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsApplicationSources ${CtsHome}/cts++ "src")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(CtsApplicationHeaders ${CtsHome}/cts++ "src")
