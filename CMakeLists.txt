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
OPTION(${OPTIONPREFIX}BUILD_TESTS "Build Tests" OFF)

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

SET(PROJECT_SOURCE_PRIVATE
	"${PROJECT_SOURCE_DIR}/source/os/async_op.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/async_op.cpp"
	"${PROJECT_SOURCE_DIR}/source/os/error.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/semaphore.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/tags.hpp"
	"${PROJECT_SOURCE_DIR}/source/os/waitable.hpp"
)

SET(PROJECT_DATA
	"${PROJECT_SOURCE_DIR}/README.md"
	"${PROJECT_SOURCE_DIR}/LICENSE"
)

SET(PROJECT_LIBRARIES
)

IF(WIN32)
	LIST(APPEND PROJECT_LIBRARIES
		advapi32
	)

	LIST(APPEND PROJECT_SOURCE_PRIVATE
		"${PROJECT_SOURCE_DIR}/source/os/windows/async_request.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/async_request.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/named-pipe.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/named-pipe.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/overlapped.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/overlapped.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/semaphore.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/semaphore.cpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/utility.hpp"
		"${PROJECT_SOURCE_DIR}/source/os/windows/utility.cpp"
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

IF(WIN32)
	# Windows
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
	
	# windows.h
	target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOGPICAPMASKS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOVIRTUALKEYCODES)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOWINMESSAGES)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOWINSTYLES)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOSYSMETRICS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMENUS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOICONS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOKEYSTATES)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOSYSCOMMANDS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NORASTEROPS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOSHOWWINDOW)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOATOM)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOCLIPBOARD)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOCOLOR)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOCTLMGR)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NODRAWTEXT)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOGDI)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOKERNEL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOUSER)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NONLS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMB)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMEMMGR)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMETAFILE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMSG)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOOPENFILE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOSCROLL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOSERVICE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOSOUND)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOTEXTMETRIC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOWH)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOWINOFFSETS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOCOMM)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOKANJI)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOHELP)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOPROFILER)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NODEFERWINDOWPOS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMCX)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOIME)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOMDI)
	target_compile_definitions(${PROJECT_NAME} PRIVATE NOINOUT)
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
