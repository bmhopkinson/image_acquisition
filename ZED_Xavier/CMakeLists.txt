cmake_minimum_required(VERSION 2.8)
project(image_acq_zedXavier)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF()

MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS_DEBUG "-O2 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")

set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

LIST(APPEND CMAKE_C_FLAGS "-Wall -Wextra -Wno-deprecated-declarations")
LIST(APPEND CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-deprecated-declarations")

# Check C++11 or C++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   add_definitions(-DCOMPILEDWITHC11)
   message(STATUS "Using flag -std=c++11.")
elseif(COMPILER_SUPPORTS_CXX0X)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   add_definitions(-DCOMPILEDWITHC0X)
   message(STATUS "Using flag -std=c++0x.")
else()
   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

#LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

find_package(Threads REQUIRED)
find_package(ZED 2 REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} EXACT REQUIRED)

add_library(system SHARED
src/system/GPIO.cpp
src/system/GPSPoller.cpp
src/system/LogUtils.cpp
)
target_include_directories(system PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  /usr/include
)
target_link_libraries(system 
  ${CMAKE_THREAD_LIBS_INIT} 
  /usr/lib/aarch64-linux-gnu/libgps.so 
  yaml-cpp
)

add_executable(logger src/logger.cpp)
target_include_directories(logger PUBLIC
 ${PROJECT_SOURCE_DIR}/include
 ${ZED_INCLUDE_DIRS}
 ${CUDA_INCLUDE_DIRS}
)
target_link_libraries(logger system ${ZED_LIBRARIES} ${CUDA_LIBRARIES})
