cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(imp-server LANGUAGES CXX)

find_package(fcl REQUIRED)

include_directories(
    third_party/eigen
    third_party/oatpp/src
    source
)

if (MSVC)
    add_compile_options(/bigobj)
endif() 

set(BUILD_SHARED_LIBS off)
set(OATPP_BUILD_TESTS off)
add_subdirectory(third_party/oatpp) 

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}") 
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/source/*.hpp)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/source/*.cpp)

add_executable(imp-server ${SOURCES} ${HEADERS})
target_link_libraries(imp-server PUBLIC oatpp fcl)
 