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

PROJECT("FoundationTest" CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

INCLUDE_DIRECTORIES(
    .
    ${CMAKE_BINARY_DIR}
    )

LINK_DIRECTORIES(../fundation ../dep/paho/src ../dep/nmealib)
set(LIBS_SYSTEM foundation paho-mqtt3a nmea)

#-std=gnu++11 
set (MY_CXX_FLAGS "-fpermissive -Wwrite-strings")
SET( CMAKE_CXX_FLAGS  "${MY_CXX_FLAGS} ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

set(SRC_LIST 
		main.cpp
	)
add_executable(FoundationTest ${SRC_LIST})
TARGET_LINK_LIBRARIES(FoundationTest pthread ${LIBS_SYSTEM}) 

INSTALL(TARGETS FoundationTest
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
