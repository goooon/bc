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

PROJECT("parse_file" C)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

INCLUDE_DIRECTORIES(
    .
	../../include
    ${CMAKE_BINARY_DIR}
    )

LINK_DIRECTORIES(../../src)
set(LIBS_SYSTEM nmea m)

set (MY_C_FLAGS "")
SET( CMAKE_C_FLAGS  "${MY_C_FLAGS} ${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

set(SRC_LIST 
		main.c
	)
add_executable(parse_file ${SRC_LIST})
TARGET_LINK_LIBRARIES(parse_file ${LIBS_SYSTEM}) 

INSTALL(TARGETS parse_file
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
