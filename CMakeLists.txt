cmake_minimum_required(VERSION 2.8.7)
project(logger CXX)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# pthread
if(UNIX) # include linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

option(build_tests "Build all of own tests." OFF)
option(build_examples "Build example programs." OFF)

### Library
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)
file(GLOB source_files ${PROJECT_SOURCE_DIR}/src/*.cpp)
# shared and static libraries
add_library(${PROJECT_NAME} SHARED ${source_files})
add_library(${PROJECT_NAME}_static STATIC ${source_files})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_static PROPERTIES PREFIX "lib")

### Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
file(GLOB header_files ${PROJECT_SOURCE_DIR}/src/*.h)
install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

### Test
if(build_tests)
    enable_testing()
    add_subdirectory(test)
endif()

### Example
if(build_examples)
    add_subdirectory(example)
endif()
