cmake_minimum_required(VERSION 3.9)
project(VIOSOWarpBlend VERSION 1.2.4 DESCRIPTION "VIOSOWarpBlend library")

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
  message(ERROR "GLUT not found!")
endif(NOT GLUT_FOUND)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
  message(ERROR "OPENGL not found!")
endif(NOT OPENGL_FOUND)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
add_definitions(${GLEW_DEFINITIONS})
if(NOT GLEW_FOUND)
  message(ERROR "GLEW not found!")
endif(NOT GLEW_FOUND)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(DEFAULT_FLAGS "-std=c++11 -Wall -Werror -Wno-unused-result -Wno-unknown-pragmas")

set(SOURCES
    "VIOSOWarpBlend/GL/GLWarpBlend.cpp"
    "VIOSOWarpBlend/LoadDAE.cpp"
    "VIOSOWarpBlend/LoadVWF.cpp"
    "VIOSOWarpBlend/logging.cpp"
    "VIOSOWarpBlend/PathHelper.cpp"
    "VIOSOWarpBlend/tinyxml2.cpp"
    "VIOSOWarpBlend/VIOSOWarpBlend.cpp"
    "Include/EyePointProvider.h"
    "Include/VIOSOWarpBlend.h"
    "Include/VWBDef.h"
    "Include/VWBTypes.h"
    "VIOSOWarpBlend/GL/GLext.h"
    "VIOSOWarpBlend/GL/GLWarpBlend.h"
    "VIOSOWarpBlend/GL/pixelshader.h"
    "VIOSOWarpBlend/common.h"
    "VIOSOWarpBlend/LoadDAE.h"
    "VIOSOWarpBlend/LoadVWF.h"
    "VIOSOWarpBlend/logging.h"
    "VIOSOWarpBlend/mac_compat.h"
    "VIOSOWarpBlend/mmath.h"
    "VIOSOWarpBlend/PathHelper.h"
    "VIOSOWarpBlend/Platform.h"
    "VIOSOWarpBlend/resource.h"
    "VIOSOWarpBlend/SPtr.h"
    "VIOSOWarpBlend/tinyxml2.h"
    "VIOSOWarpBlend/linux_compat.h")

add_library(VIOSOWarpBlend SHARED ${SOURCES})
set_target_properties(VIOSOWarpBlend PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
#target_link_libraries(analysis_lib movinghead_lib thrift ${CERES_LIBRARIES})
