cmake_minimum_required(VERSION 3.5)
project(test_windows_direct-read-write_mt)

SET(PROJECT_SOURCES
	"${PROJECT_SOURCE_DIR}/main.cpp"
	"${PROJECT_SOURCE_DIR}/../../common.cpp"
	"${PROJECT_SOURCE_DIR}/../../common.hpp"
)

SET(PROJECT_LIBRARIES
)

# Includes
include_directories(
	${PROJECT_SOURCE_DIR}
)

# Building
ADD_EXECUTABLE(${PROJECT_NAME}
	${PROJECT_SOURCES}
)

# Linking
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	lib-datalane
	${PROJECT_LIBRARIES}
)

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
	#target_compile_definitions(${PROJECT_NAME} PRIVATE NONLS)
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