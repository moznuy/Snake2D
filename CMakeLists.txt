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
include_directories(${SDL2_INCLUDE_DIR})

include(CTest)
enable_testing()

file(GLOB sources "*.cpp" "*.h")

add_executable(Snake2D ${sources})


# OS and compiler checks.
if(UNIX)
    MESSAGE("Unix detected")
    # linux / normal unix
    add_definitions(-DLINUX)
    if(CYGWIN)
        # Special Cygwin stuff here
    elseif(APPLE)
        # Special Apple stuff here
        remove_definitions(-DLINUX)
        add_definitions(-DDARWIN)
    endif()
elseif(WIN32)
    MESSAGE("Windows detected")
    add_definitions(-DWIN32)
    SET(PROJECT_LIBS Ws2_32.lib)
    if(MINGW)
        # Special MINGW stuff here
    elseif(MSVC)
        # Special MSVC stuff here
        add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    else()
        # No idea what it is, but there's a good chance it's too weird.
        MESSAGE( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

target_link_libraries(Snake2D ${SDL2_LIBRARY} Threads::Threads ${PROJECT_LIBS})
#configure_file(${PROJECT_SOURCE_DIR}/cmake/Header.h.in ${PROJECT_SOURCE_DIR}/Header.h)

include(CPack)
