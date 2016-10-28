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

/**
 * Extended descriptions of sentences are taken from
 *   http://www.gpsinformation.org/dale/nmea.htm
 */

#ifndef __NMEALIB_GPZDA_H__
#define __NMEALIB_GPZDA_H__

#include <nmealib/info.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/** The NMEA prefix */
#define NMEALIB_GPZDA_PREFIX "GPZDA"

/**
 * GPZDA packet information structure (Date and Time)
 *
 * date and time
 *
 * <pre>
 * $GPZDA,hhmmss.ss,dd,mm,yyyy,xx,yy*checksum
 * </pre>
 *
 * | Field       | Description                                            | present        |
 * | :---------: | ------------------------------------------------------ | :------------: |
 * | $GPZDA      | NMEA prefix                                            | -              |
 * | hhmmss.ss   | hrminsec.msec                                          | UTCTIME        |
 * | dd  		 | day                           						  | day (1)        |
 * | mm  		 | month                         						  | month (1)      |
 * | yyyy  		 | year                           						  | year (1)       |
 * | xx          | local zone hours -13..13								  | local hours    |
 * | xx          | local zone minutes 0..59								  | local min      |
 * | checksum    | NMEA checksum                                          | -              |
 *
 * (4) Not supported yet<br/>
 * (5) Supported formats: HHMMSS.SS<br/>
 *
 * Example:
 *
 * <pre>
 * $GPZDA,082710.00,16,09,2002,00,00*64
 * </pre>
 *
 */
typedef struct _NmeaGPZDA {
  uint32_t     present;
  NmeaTime     utc;
  int          local_hours;
  unsigned int local_minutes;
} NmeaGPZDA;

/**
 * Parse a GPZDA sentence
 *
 * @param s The sentence
 * @param sz The length of the sentence
 * @param pack Where the result should be stored
 * @return True on success
 */
bool nmeaGPZDAParse(const char *s, const size_t sz, NmeaGPZDA *pack);

/**
 * Update an unsanitised NmeaInfo structure from a GPZDA packet structure
 *
 * @param pack The GPZDA packet structure
 * @param info The unsanitised NmeaInfo structure
 */
void nmeaGPZDAToInfo(const NmeaGPZDA *pack, NmeaInfo *info);

/**
 * Convert a sanitised NmeaInfo structure into a NmeaGPZDA structure
 *
 * @param info The sanitised NmeaInfo structure
 * @param pack The NmeaGPZDA structure
 */
void nmeaGPZDAFromInfo(const NmeaInfo *info, NmeaGPZDA *pack);

/**
 * Generate a GPZDA sentence
 *
 * @param s The buffer to generate the sentence in
 * @param sz The size of the buffer
 * @param pack The NmeaGPZDA structure
 * @return The length of the generated sentence; less than zero on failure,
 * larger than sz when the size of the buffer is too small to generate the
 * sentence in
 */
size_t nmeaGPZDAGenerate(char *s, const size_t sz, const NmeaGPZDA *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GPZDA_H__ */

