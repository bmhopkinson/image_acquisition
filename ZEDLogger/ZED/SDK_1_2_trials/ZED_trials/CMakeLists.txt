SET(execName ZED_TRIALS)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if (COMMAND cmake_policy)
   cmake_policy(SET CMP0003 OLD)
   cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

SET(EXECUTABLE_OUTPUT_PATH ".")

IF(WIN32)
SET(ZED_INCLUDE_DIRS $ENV{ZED_INCLUDE_DIRS})
	if (CMAKE_CL_64) #64 bits
	   SET(ZED_LIBRARIES $ENV{ZED_LIBRARIES_64})
        else (CMAKE_CL_64) #32 bits
	     message("32bits compilation is not longer avaiable with CUDA 7.0")
	endif (CMAKE_CL_64)
SET(ZED_LIBRARY_DIRECTORY $ENV{ZED_LIBRARY_DIR})
SET(OPENCV_DIR $ENV{OPENCV_DIR})
find_package(CUDA 8.0 REQUIRED)
ELSE() #linux
find_package(ZED REQUIRED)
find_package(CUDA 8.0 REQUIRED)
ENDIF(WIN32)
find_package(OpenCV 2.4 COMPONENTS core highgui imgproc REQUIRED)
SET(CUDA_USE_STATIC_CUDA_RUNTIME OFF)

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

link_directories(${ZED_LIBRARY_DIR})
link_directories(${OpenCV_LIBRARY_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

SET(SRC_FOLDER src)
FILE(GLOB_RECURSE SRC_FILES "${SRC_FOLDER}/*.cpp")

ADD_EXECUTABLE(${execName} ${SRC_FILES})

add_definitions(-std=c++0x)

TARGET_LINK_LIBRARIES(${execName}
			${ZED_LIBRARIES}
			${OpenCV_LIBRARIES}
			${CUDA_LIBRARIES} ${CUDA_npps_LIBRARY} ${CUDA_nppi_LIBRARY}
		)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3") #release perf mode
