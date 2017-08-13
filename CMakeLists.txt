#################################################
# cmake file for building ArborContent framework
# @author Eté Rémi ,IPNL
# Copyright (c) CNRS / IPNL
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.2 FATAL_ERROR )
#################################################

# project name
PROJECT( ArborContent )

SET( ${PROJECT_NAME}_VERSION_MAJOR 01 )
SET( ${PROJECT_NAME}_VERSION_MINOR 05 )
SET( ${PROJECT_NAME}_VERSION_PATCH 01 )

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
INCLUDE( PandoraCMakeSettings )

OPTION( PANDORA_MONITORING "Whether to use PandoraMonitoring (requires ROOT)" OFF )
MESSAGE( STATUS "PANDORA_MONITORING : ${PANDORA_MONITORING}" )

OPTION( ARBOR_PARALLEL "Whether to use multithreading (requires OpenMP)" OFF )
MESSAGE( STATUS "ARBOR_PARALLEL : ${ARBOR_PARALLEL}" )

OPTION( INSTALL_DOC "Set to OFF to skip build/install Documentation" OFF )
MESSAGE( STATUS "INSTALL_DOC : ${INSTALL_DOC}" )

### SOURCE ##################################################################

#include directories
INCLUDE_DIRECTORIES( ./include )

SET( INPUT_DIRS 
     ArborApi ArborObjects ArborCheating ArborClustering ArborHelpers ArborMonitoring ArborPfoConstruction ArborParticleId 
     ArborPlugins ArborReclustering ArborTools ArborTopologicalAssociation ArborTrackClusterAssociation ArborUtility )

FILE(GLOB INPUT_DIR_INCS ./include/*.h)
SOURCE_GROUP("Header files" FILES ${INPUT_DIR_INCS} )
LIST(APPEND ArborContent_INCS ${INPUT_DIR_INCS})

FOREACH( INPUT_DIR ${INPUT_DIRS} )
    FILE(GLOB_RECURSE INPUT_DIR_INCS ./include/${INPUT_DIR}/*.h)
    SOURCE_GROUP("Header files\\${INPUT_DIR}" FILES ${INPUT_DIR_INCS} )
    LIST(APPEND ArborContent_INCS ${INPUT_DIR_INCS})

    FILE (GLOB_RECURSE INPUT_DIR_SRCS ./src/${INPUT_DIR}/*.cc )
    SOURCE_GROUP("Source files\\${INPUT_DIR}" FILES ${INPUT_DIR_SRCS} )
    LIST(APPEND ArborContent_SRCS ${INPUT_DIR_SRCS})
ENDFOREACH()

### C++11 check #############################################################

INCLUDE( CheckCXXCompilerFlag )

UNSET( COMPILER_SUPPORTS_CXX11 CACHE )
CHECK_CXX_COMPILER_FLAG( "-std=c++11" COMPILER_SUPPORTS_CXX11 )

IF( NOT CMAKE_CXX_FLAGS )
    SET( CMAKE_CXX_FLAGS "-std=c++11" )
    SET( PROCEED_WITHOUT_CXX_FLAGS ON )
ENDIF()

UNSET( COMPILER_SUPPORTS_CXX_FLAGS CACHE )
CHECK_CXX_COMPILER_FLAG( ${CMAKE_CXX_FLAGS} COMPILER_SUPPORTS_CXX_FLAGS )
IF( NOT COMPILER_SUPPORTS_CXX_FLAGS )
    MESSAGE( STATUS "The compiler ${CMAKE_CXX_COMPILER} does not support cxx flags ${CMAKE_CXX_FLAGS}" )
    IF( NOT PROCEED_WITHOUT_CXX_FLAGS )
        MESSAGE( FATAL_ERROR "CMake abort." )
    ENDIF()
    MESSAGE( STATUS "Proceed without cxx flags ${CMAKE_CXX_FLAGS}" )
    UNSET( CMAKE_CXX_FLAGS )
ENDIF()

if(APPLE)
   SET( CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Wno-long-long -Wno-sign-compare -Wshadow -fno-strict-aliasing ${CMAKE_CXX_FLAGS}" )
else()
   SET( CMAKE_CXX_FLAGS "-Wall -Wextra -ansi -pedantic -Wno-long-long -Wno-sign-compare -Wshadow -fno-strict-aliasing ${CMAKE_CXX_FLAGS}" )
endif()



# -------------- packages -------------
FIND_PACKAGE( PandoraSDK 02.00.00 REQUIRED NO_CMAKE_PATH )
INCLUDE_DIRECTORIES( ${PandoraSDK_INCLUDE_DIRS} )
LINK_LIBRARIES( ${PandoraSDK_LIBRARIES} )
ADD_DEFINITIONS ( ${PandoraSDK_DEFINITIONS} )


FIND_PACKAGE( ROOT 5.26.00 REQUIRED COMPONENTS Core Tree Physics Matrix Eve Geom RGL EG TMVA ) # minimum required ROOT version
INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIRS} )
LINK_LIBRARIES( ${ROOT_LIBRARIES} ${ROOT_COMPONENT_LIBRARIES} )
ADD_DEFINITIONS ( ${ROOT_DEFINITIONS} )


IF( PANDORA_MONITORING )
  FIND_PACKAGE( PandoraMonitoring REQUIRED NO_CMAKE_PATH )
  INCLUDE_DIRECTORIES( ${PandoraMonitoring_INCLUDE_DIRS} )
  LINK_LIBRARIES( ${PandoraMonitoring_LIBRARIES} )
  ADD_DEFINITIONS ( ${PandoraMonitoring_DEFINITIONS} )
  ADD_DEFINITIONS( "-DMONITORING" )
ENDIF()


IF( ARBOR_PARALLEL )
    FIND_PACKAGE( OpenMP REQUIRED )
    ADD_DEFINITIONS( ${OpenMP_CXX_FLAGS} )
    ADD_DEFINITIONS( "-DARBOR_PARALLEL=1" )
ENDIF()


IF( INSTALL_DOC AND EXISTS "${PROJECT_SOURCE_DIR}/doc/CMakeLists.txt" )
    ADD_SUBDIRECTORY( ./doc )
ELSE()
    MESSAGE( STATUS "INSTALL_DOC set to OFF" )
    SET( INSTALL_DOC OFF )
ENDIF()

### LIBRARY #################################################################

# add library
ADD_LIBRARY( ${PROJECT_NAME} SHARED ${ArborContent_SRCS} ${ArborContent_INCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})

IF( ARBOR_PARALLEL )
    TARGET_LINK_LIBRARIES( ${PROJECT_NAME} gomp )
ENDIF()


### INSTALL #################################################################

# install library
INSTALL( TARGETS ${PROJECT_NAME} DESTINATION lib COMPONENT Runtime )

# install header files
INSTALL( DIRECTORY include/ DESTINATION include )

# display some variables and write them to cache
PANDORA_DISPLAY_STD_VARIABLES()

# generate and install following configuration files
PANDORA_GENERATE_PACKAGE_CONFIGURATION_FILES( ${PROJECT_NAME}Config.cmake ${PROJECT_NAME}ConfigVersion.cmake ${PROJECT_NAME}LibDeps.cmake )

