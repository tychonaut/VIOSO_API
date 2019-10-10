cmake_minimum_required(VERSION 3.9)
project(VIOSOWarpBlend VERSION 1.2.4 DESCRIPTION "VIOSOWarpBlend library")



#find_package(GLUT REQUIRED
#	PATHS ${CMAKE_SOURCE_DIR}/freeglut
#)

############################################ GLUT OWN HACK ####
 #GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
 # GLUT_LIBRARIES, the libraries to link against
 # GLUT_FOUND, If false, do not try to use GLUT.
find_path( GLUT_INCLUDE_DIRS
	NAMES GL/glut.h
    PATHS  ${CMAKE_SOURCE_DIR}/freeglut/include )
find_library( 
	GLUT_LIBRARIES 
	NAMES 
		glut glut32 freeglut
    PATHS
		${CMAKE_SOURCE_DIR}/freeglut/
		${CMAKE_SOURCE_DIR}/freeglut/lib
		${CMAKE_SOURCE_DIR}/freeglut/lib/x64
    )
set(GLUT_FOUND true)


include_directories(${GLUT_INCLUDE_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
  message(ERROR "GLUT not found!")
endif(NOT GLUT_FOUND)



############################################ OpenGL 
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
  message(ERROR "OPENGL not found!")
endif(NOT OPENGL_FOUND)



############################################ GLEW OWN HACK ####
#find_package(GLEW REQUIRED
#	PATHS ${CMAKE_SOURCE_DIR}/glew
#)
find_path( GLEW_INCLUDE_DIRS
	NAMES GL/glew.h
    PATHS  ${CMAKE_SOURCE_DIR}/glew/include )
#hardcode to use static release always
find_library(
	GLEW_STATIC_LIBRARY_RELEASE
             NAMES 
				GLEW glew glew32s
             PATH_SUFFIXES 
				lib lib64 libx32 lib/Release/${_arch}
             PATHS 
				${CMAKE_SOURCE_DIR}/glew/
				ENV GLEW_ROOT
)
set(GLEW_STATIC_LIBRARIES ${GLEW_STATIC_LIBRARY_RELEASE})
set(GLEW_LIBRARIES ${GLEW_STATIC_LIBRARIES})
			 
set(GLEW_FOUND true)
set(GLEW_VERSION "2.1.0")
set(GLEW_VERSION_MAJOR 2)
set(GLEW_VERSION_MINOR 1)
set(GLEW_VERSION_MICRO 0)

			 
		 


include_directories(${GLEW_INCLUDE_DIRS})
add_definitions(${GLEW_DEFINITIONS})
if(NOT GLEW_FOUND)
  message(ERROR "GLEW not found!")
endif(NOT GLEW_FOUND)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(DEFAULT_FLAGS "-std=c++11 -Wall -Werror -Wno-unused-result -Wno-unknown-pragmas")

set(SOURCES
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/GL/GLWarpBlend.cpp"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/LoadDAE.cpp"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/LoadVWF.cpp"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/logging.cpp"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/PathHelper.cpp"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/3rdParty/tinyxml2/tinyxml2.cpp"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/VIOSOWarpBlend.cpp"
    "${CMAKE_SOURCE_DIR}/Include/EyePointProvider.h"
    "${CMAKE_SOURCE_DIR}/Include/VIOSOWarpBlend.h"
    "${CMAKE_SOURCE_DIR}/Include/VWBDef.h"
    "${CMAKE_SOURCE_DIR}/Include/VWBTypes.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/GL/GLext.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/GL/GLWarpBlend.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/GL/pixelshader.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/common.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/LoadDAE.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/LoadVWF.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/logging.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/mac_compat.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/mmath.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/PathHelper.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/Platform.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/resource.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/SPtr.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/3rdParty/tinyxml2/tinyxml2.h"
    "${CMAKE_SOURCE_DIR}/VIOSOWarpBlend/linux_compat.h")

add_library(VIOSOWarpBlend SHARED ${SOURCES})
set_target_properties(VIOSOWarpBlend PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
#target_link_libraries(analysis_lib movinghead_lib thrift ${CERES_LIBRARIES})
