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
PROJECT("struct _NmeaGenerator *gen" C)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

## compilation/linkage settings
INCLUDE_DIRECTORIES(
    .
    ./include
    ${CMAKE_BINARY_DIR}
    )

SET(common_src
	src/context.c
	src/generator.c
	src/gpgga.c
	src/gpgsa.c
	src/gpgsv.c
	src/gprmc.c
	src/gpvtg.c
	src/info.c
	src/nmath.c
	src/parser.c
	src/sentence.c
	src/util.c
	src/validate.c
    )

set (MY_C_FLAGS "")
SET( CMAKE_C_FLAGS  "${MY_C_FLAGS} ${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(LIBS_SYSTEM ws2_32)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(LIBS_SYSTEM dl)
ENDIF()

ADD_LIBRARY(nmea SHARED ${common_src})
TARGET_LINK_LIBRARIES(nmea ${LIBS_SYSTEM}) 
SET_TARGET_PROPERTIES(
    nmea PROPERTIES
    VERSION 1.0
    SOVERSION 1.0.1)
INSTALL(TARGETS nmea
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)

ADD_SUBDIRECTORY(samples/generate)
ADD_SUBDIRECTORY(samples/generator)
ADD_SUBDIRECTORY(samples/math)
ADD_SUBDIRECTORY(samples/parse)
ADD_SUBDIRECTORY(samples/parse_file)
ADD_SUBDIRECTORY(samples/parse_serial)


