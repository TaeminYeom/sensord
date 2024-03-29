CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sensor-physical CXX)
INCLUDE(GNUInstallDirs)

SET(ACCEL "OFF")

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/src/shared
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Common Options
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -fomit-frame-pointer -std=gnu++0x")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--print-gc-sections")
MESSAGE("FLAGS: ${CMAKE_CXX_FLAGS}")
MESSAGE("FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# Internal Debugging Options
#ADD_DEFINITIONS(-Wall -g -D_DEBUG)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PLUGINS_PKGS REQUIRED dlog glib-2.0 vconf libtzplatform-config)

FOREACH(flag ${PLUGINS_PKGS_CFLAGS})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

FILE(GLOB SRCS *.cpp)

IF("${ACCEL}" STREQUAL "ON")
FILE(GLOB_RECURSE SRCS ${SRCS} accel/*.cpp)
ADD_DEFINITIONS(-DENABLE_ACCEL)
ENDIF()

MESSAGE("Sources: ${SRCS}")
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PLUGINS_PKGS_LDFLAGS} "sensord-shared")
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/sensor/physical)
