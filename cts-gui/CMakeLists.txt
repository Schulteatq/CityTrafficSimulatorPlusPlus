project(cts-gui LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
)

file(GLOB_RECURSE Headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/cts-gui/*.h
)


# Define an executable
add_library(cts-gui ${Sources} ${Headers})
target_include_directories(cts-gui
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Define the libraries this project depends upon
target_link_libraries(cts-gui PRIVATE cts-core Qt5::Widgets)

if(ENABLE_SCRIPTING)
  target_link_libraries(cts-gui PRIVATE cts-lua)
endif()

set_target_properties(cts-gui PROPERTIES DEFINE_SYMBOL "CTS_GUI_EXPORTS")
if(BUILD_SHARED_LIBS)
  target_compile_definitions(cts-gui PUBLIC "-DCTS_GUI_BUILD_SHARED_LIBS")
endif()


DEFINE_SOURCE_GROUPS_FROM_SUBDIR(Sources ${CtsHome}/cts-gui "src")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(Headers ${CtsHome}/cts-gui "include/cts-gui")
