cmake_minimum_required(VERSION 3.5)
project(lib-datalane VERSION 0.0.1)

# Test if we are a subproject
SET(OPTIONPREFIX "lib-datalane_")
IF(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	SET(OPTIONPREFIX "")
ENDIF()

# Detect Architecture (Bitness)
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

################################################################################
# Settings
################################################################################
# Static or Dynamic?
OPTION(${OPTIONPREFIX}BUILD_STATIC "Build Static Library" ON)
OPTION(${OPTIONPREFIX}BUILD_MODULE "Build Module Library instead of Dynamic Library" OFF)

# Tests
OPTION(${OPTIONPREFIX}BUILD_SAMPLES "Build Samples" OFF)
OPTION(${OPTIONPREFIX}BUILD_TESTS "Build Tests" ON)

################################################################################
# System & Utilities
################################################################################
# Detect Libary Suffix
IF(WIN32)
	SET(libExtension ".dll")
ELSEIF(APPLE)
	SET(libExtension ".dylib")
ELSE()
    SET(libExtension ".so")
ENDIF()

################################################################################
# Source
################################################################################
SET(PROJECT_SOURCE_PUBLIC
	"${PROJECT_SOURCE_DIR}/include/datalane.hpp"
	"${PROJECT_SOURCE_DIR}/include/datalane-socket.hpp"
	"${PROJECT_SOURCE_DIR}/include/datalane-error.hpp"
)

SET(PROJECT_SOURCE_PRIVATE
	"${PROJECT_SOURCE_DIR}/source/datalane.cpp"
	"${PROJECT_SOURCE_DIR}/source/datalane-socket.cpp"
	"${PROJECT_SOURCE_DIR}/source/datalane-socket-client.hpp"
	"${PROJECT_SOURCE_DIR}/source/datalane-socket-client.cpp"
	"${PROJECT_SOURCE_DIR}/source/datalane-socket-server.hpp"
	"${PROJECT_SOURCE_DIR}/source/datalane-socket-server.cpp"
	"${PROJECT_SOURCE_DIR}/source/os/error.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/tags.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/waitable.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/async_op.hpp"
)

SET(PROJECT_DATA
	"${PROJECT_SOURCE_DIR}/README.md"
	"${PROJECT_SOURCE_DIR}/LICENSE"
)

SET(PROJECT_LIBRARIES
)

IF(WIN32)
	# Windows
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
	LIST(APPEND PROJECT_LIBRARIES
		advapi32
	)

	LIST(APPEND PROJECT_SOURCE_PRIVATE
		"${PROJECT_SOURCE_DIR}/source/os/windows/async_request.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/async_request.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/named-pipe.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/named-pipe.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/semaphore.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/semaphore.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/overlapped.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/overlapped.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/waitable.cpp"
	)
ELSEIF(APPLE)
	# MacOSX

	LIST(APPEND PROJECT_SOURCE_PRIVATE
	)
ELSEIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	# Linux

	LIST(APPEND PROJECT_SOURCE_PRIVATE
	)
ELSEIF("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
	# FreeBSD

	LIST(APPEND PROJECT_SOURCE_PRIVATE
	)
ENDIF()

# Grouping
source_group("Data Files" FILES $lib-datalane_DATA)
source_group(TREE "${PROJECT_SOURCE_DIR}/source" PREFIX "Source" FILES ${PROJECT_SOURCE_PRIVATE})
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Include" FILES ${PROJECT_SOURCE_PUBLIC})

################################################################################
# Building
################################################################################

# Building
IF(${OPTIONPREFIX}BUILD_STATIC)
	ADD_LIBRARY(${PROJECT_NAME} STATIC
		${PROJECT_SOURCE_PRIVATE}
		${PROJECT_SOURCE_PUBLIC}
	)
ELSEIF(${OPTIONPREFIX}BUILD_MODULE)
	ADD_LIBRARY(${PROJECT_NAME} MODULE
		${PROJECT_SOURCE_PRIVATE}
		${PROJECT_SOURCE_PUBLIC}
	)
ELSE()
	ADD_LIBRARY(${PROJECT_NAME} SHARED
		${PROJECT_SOURCE_PRIVATE}
		${PROJECT_SOURCE_PUBLIC}
	)
ENDIF()

# Includes
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
	PRIVATE source
	PUBLIC include
)

# Directories
LINK_DIRECTORIES(
)

# Linking
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	${PROJECT_LIBRARIES}
)

################################################################################
# Samples
################################################################################
IF(${OPTIONPREFIX}BUILD_SAMPLES)
	ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/samples)
ENDIF(${OPTIONPREFIX}BUILD_SAMPLES)

################################################################################
# Tests
################################################################################
IF(${OPTIONPREFIX}BUILD_TESTS)
	ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/tests)
ENDIF(${OPTIONPREFIX}BUILD_TESTS)
