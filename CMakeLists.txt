cmake_minimum_required(VERSION 3.0.0)
project(Snake2D VERSION 0.1.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include(CTest)
enable_testing()

file(GLOB sources "*.cpp" "*.h")

add_executable(Snake2D ${sources})
target_link_libraries(Snake2D ${SDL2_LIBRARY} Threads::Threads)


#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

configure_file(${PROJECT_SOURCE_DIR}/cmake/Header.h.in ${PROJECT_SOURCE_DIR}/Header.h)

include(CPack)
