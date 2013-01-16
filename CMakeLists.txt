cmake_minimum_required(VERSION 2.8) 

project(oxygen.jsx)

set(CMAKE_BUILD_TYPE debug)
#set(BINARY_DIR build)

set(JSX ${PROJECT_SOURCE_DIR}/../jsx)

set(EXECUTABLE_OUTPUT_PATH ${JSX}/bin)
set(LIBRARY_OUTPUT_PATH ${JSX}/bin)
set(CMAKE_CURRENT_BINARY_DIR ${JSX}/bin)

set(CMAKE_SHARED_LINKER_FLAG "${CMAKE_SHARED_LINKER_FLAGS} lt_sys_lib_dlsearch_path_spec=${EXECUTABLE_OUTPUT_PATH}")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS "-Wno-parentheses-equality -DNDEBUG")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CMAKE_CXX_FLAGS "-std=c++0x -fPIC -DNDEBUG")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


set(EXTERN ${JSX}/extern)


include_directories(${EXTERN})

include_directories(${EXTERN}/boost/include)
link_directories(${EXTERN}/boost/lib)

include_directories(${EXTERN}/v8/include)
link_directories(${EXTERN}/b8/lib)

include_directories(${EXTERN}/v8-juice/src/include)
link_directories(${EXTERN}/v8-juice/lib)

include_directories(${JSX}/sdk/core/src)
link_directories(${JSX}/bin)

set(MATH ${PROJECT_SOURCE_DIR}/../math)
include_directories(${MATH}/src)

file(GLOB SRC "src/*.cpp")

#ADD_PCH_RULE(core.hpp ${SRC})

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#set(CMAKE_CURRENT_BINARY_DIR "${PROJECT_BINARY_DIR}/bin")

set(BOOST_LIBRARIES boost_thread boost_filesystem boost_system boost_iostreams)
#set(PION_LIBRARIES pion)
set(V8_LIBRARIES v8 v8-juice)

add_library(oxygen.jsx SHARED ${SRC})

target_link_libraries(oxygen.jsx core.jsx v8 v8-juice ${BOOST_LIBRARIES} X11 Xrandr)
#pthread dl rt zmq cryptopp libjson stringencoders ssl crypt pion v8 v8-juice ${BOOST_LIBRARIES})

#add_dependencies(core.jsx zlib.extern libzmq.extern cryptopp.extern stringencoders.extern libjson.extern v8-juice.extern pion.extern)

#add_dependencies(oxygen.jsx X11 Xrandr)