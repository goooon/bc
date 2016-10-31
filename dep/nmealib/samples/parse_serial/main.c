/*
 * This file is part of nmealib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nmealib/context.h>
#include <nmealib/info.h>
#include <nmealib/nmath.h>
#include <nmealib/parser.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <libgen.h>
#include <unistd.h>
#include "serial.h"
#endif

static const char * traceStr = "Trace: ";
static const char * errorStr = "Error: ";
static const char * eol = "\n";

static void trace(const char *str, size_t str_size) {
  write(1, traceStr, strlen(traceStr));
  write(1, str, str_size);
  write(1, eol, strlen(eol));
}
static void error(const char *str, size_t str_size) {
  write(1, errorStr, strlen(errorStr));
  write(1, str, str_size);
  write(1, eol, strlen(eol));
}

static void printDetailInfo(NmeaInfo *info)
{
	double db = 0;

	printf("\r\n\r\n");
	printf("====================================\n");
	printf("utc: %4d/%02d/%02d-%02d:%02d:%02d:%04d\n",
		info->utc.year, info->utc.mon, info->utc.day,
		info->utc.hour, info->utc.min, info->utc.sec, info->utc.hsec);
	printf("localtime: %4d/%02d/%02d-%02d:%02d:%02d:%04d\n",
		info->localtime.year, info->localtime.mon, info->localtime.day,
		info->localtime.hour, info->localtime.min, info->localtime.sec, info->localtime.hsec);

	printf("longitude: %f %C\n", info->longitude, (info->longitude < db) ? 'W' : 'E');
	printf("latitude: %f %C\n", info->latitude, (info->latitude < db) ? 'S' : 'N');

	printf("elevation: %f(M)\n", info->elevation);
	printf("speed: %f(kph)\n", info->speed);
	printf("track: %f\n", info->track);

	printf("sig: %s\n", nmeaInfoSignalToString(info->sig));
	printf("fix: %s\n", nmeaInfoFixToString(info->fix));

	printf("unused satellites: %d\n", info->satellites.inViewCount);
	printf("used satellites: %d\n", info->satellites.inUseCount);
	
	printf("pdop: %f\n", info->pdop);
	printf("hdop: %f\n", info->hdop);
	printf("vdop: %f\n", info->vdop);
}

int main(int argc, char *argv[]) {
	NmeaInfo info;
	NmeaParser parser;
	char buff[2048];
	size_t it = 0, size = 0;
	int ret;
	serial_t serial;
	char *devname;
	uint32_t br = 9600;

	if (argc <= 1) {
		devname = "/dev/ttySAC2";
	} else {
		devname = argv[1];
		if (argc >= 3) {
			br = (uint32_t)atoi(argv[2]);
		}
	}

	if (serial_open(&serial, devname, br) < 0) {
		fprintf(stderr, "serial_open(): %s\n", serial_errmsg(&serial));
		exit(1);
	}

	serial_set_baudrate(&serial, br);
	serial_set_databits(&serial, 8);
	serial_set_stopbits(&serial, 1);
	serial_set_parity(&serial, PARITY_NONE);
	serial_set_xonxoff(&serial, 0);
	
	nmeaContextSetTraceFunction(&trace);
	nmeaContextSetErrorFunction(&error);

	nmeaInfoClear(&info);
	nmeaParserInit(&parser, 0);

	while ((size = serial_read(&serial, buff, sizeof(buff), 1000)) >= 0) {
		if (size > 0) {
			nmeaParserParse(&parser, &buff[0], size, &info);
			printDetailInfo(&info);
		} else {
			printf("nothing data from %s\n", devname);
		}
	}

	nmeaParserDestroy(&parser);
	serial_close(&serial);

	return 0;
}
