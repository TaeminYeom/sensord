CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sensord-main CXX)
INCLUDE(GNUInstallDirs)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

# Common Options
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -omit-frame-pointer -std=gnu++0x")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-section -Wl,--print-gc-section")
MESSAGE("FLAGS: ${CMAKE_CXX_FLAGS}")
MESSAGE("FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# Internal Debugging Options
#add_definitions(-Wall -g -D_DEBUG)

# Sub-directory
ADD_SUBDIRECTORY(src/shared)
ADD_SUBDIRECTORY(src/server)
ADD_SUBDIRECTORY(src/client)
ADD_SUBDIRECTORY(src/client-dummy)
ADD_SUBDIRECTORY(src/sensorctl)

INSTALL(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sensor
	FILES_MATCHING PATTERN "*.h"
)
