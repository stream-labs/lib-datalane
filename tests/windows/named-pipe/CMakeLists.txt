cmake_minimum_required(VERSION 3.5)
project(test_windows_named_pipe)

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

file(GENERATE 
    OUTPUT "${PROJECT_SOURCE_DIR}/includes.txt" 
    CONTENT "$<TARGET_PROPERTY:test_windows_named_pipe,INCLUDE_DIRECTORIES>\n"
)