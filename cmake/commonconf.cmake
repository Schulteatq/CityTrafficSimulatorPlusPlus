if(NOT CommonconfProcessed)

include("cmake/utils.cmake")

set(CtsHome ${CMAKE_CURRENT_SOURCE_DIR})


if(WIN32)
  # Disable warnings for Microsoft compiler:
  # C4251  class needs to have dll interface (used for std classes)
  # C4503: The decorated name was longer than the compiler limit (4096), and was truncated.
  #        Happens with some Eigen includes.
  # C4068: Unknown Pragma since we use some GCC pragmas in the code.
  add_definitions(/wd4251 /wd4503 /wd4068 /W3)

  # enable parallel builds in Visual Studio
  add_definitions(/MP)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif(WIN32)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt5Widgets REQUIRED)

set(CommonconfProcessed TRUE)
endif(NOT CommonconfProcessed)
