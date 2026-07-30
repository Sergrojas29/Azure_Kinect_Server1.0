cmake_minimum_required(VERSION 3.20)
project(Azure_Kinect_Server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find System Dependencies
find_package(PkgConfig REQUIRED)

# Use IMPORTED_TARGET to automatically handle both includes and libraries
pkg_check_modules(GPHOTO2 REQUIRED IMPORTED_TARGET libgphoto2)
pkg_check_modules(CUPS REQUIRED IMPORTED_TARGET cups)

find_package(Threads REQUIRED)
find_package(k4a REQUIRED)
find_package(k4abt REQUIRED)

find_package(OpenMP REQUIRED)
find_package(TBB REQUIRED)


include(FetchContent)

# Crow
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        master 
)
FetchContent_MakeAvailable(Crow)


# CPR (HTTP Client)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.10.4 
)
set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "" FORCE) 
set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)     
FetchContent_MakeAvailable(cpr)

add_executable(azure_kinect_server 
    src/main.cpp
    src/Kinect.cpp
    src/PoseChecker.cpp
    src/utils.cpp
)

# 5. Include Paths
target_include_directories(azure_kinect_server PRIVATE 
    include
)

# 6. Link Libraries
target_link_libraries(azure_kinect_server PRIVATE
    k4a::k4a 
    k4abt::k4abt  
    PkgConfig::GPHOTO2
    PkgConfig::CUPS
    ${OpenCV_LIBS}
    Threads::Threads
    Crow::Crow 
    cpr::cpr 
    OpenMP::OpenMP_CXX
    TBB::tbb
)