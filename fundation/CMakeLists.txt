#*******************************************************************************
#  Copyright (c) 2015 logi.cals GmbH
# 
#  All rights reserved. This program and the accompanying materials
#  are made available under the terms of the Eclipse Public License v1.0
#  and Eclipse Distribution License v1.0 which accompany this distribution. 
# 
#  The Eclipse Public License is available at 
#     http://www.eclipse.org/legal/epl-v10.html
#  and the Eclipse Distribution License is available at 
#    http://www.eclipse.org/org/documents/edl-v10.php.
# 
#  Contributors:
#     Rainer Poisel - initial version
#*******************************************************************************/

# Note: on OS X you should install XCode and the associated command-line tools
PROJECT("foundation" CXX)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

## compilation/linkage settings
INCLUDE_DIRECTORIES(
    .
    ${CMAKE_BINARY_DIR}
    )

SET(common_src
    src/cpp/core/Types.cpp
	src/cpp/util/SharedMemory.cpp
	src/cpp/util/Semaphore.cpp
	src/cpp/util/Thread.cpp
	src/cpp/util/Tree.cpp
	src/cpp/util/Heap.cpp
	src/cpp/util/StackTrace.cpp
	src/cpp/util/Log.cpp
	src/cpp/util/LinkedList.cpp
	src/cpp/util/Pipe.cpp
	src/cpp/bcp.cpp
	src/cpp/bcp_comm.cpp
	src/cpp/bcp_packet.cpp
	src/cpp/binary_formater.cpp
	src/cpp/crc32.cpp
    )

set (MY_CXX_FLAGS "-fpermissive -std=gnu++11 -Wwrite-strings")
SET( CMAKE_CXX_FLAGS  "${MY_CXX_FLAGS} ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(LIBS_SYSTEM ws2_32)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(LIBS_SYSTEM dl)
ENDIF()

ADD_LIBRARY(foundation SHARED ${common_src})
TARGET_LINK_LIBRARIES(foundation pthread ${LIBS_SYSTEM}) 
SET_TARGET_PROPERTIES(
    foundation PROPERTIES
    VERSION 1.0
    SOVERSION 1.0.1)
INSTALL(TARGETS foundation
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
