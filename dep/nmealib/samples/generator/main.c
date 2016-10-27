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

#include <nmealib/generator.h>
#include <nmealib/info.h>
#include <nmealib/sentence.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static void my_sleep(int miliseconds)
{
	Sleep(miliseconds);
}
#else
#include <unistd.h>
static void my_sleep(int miliseconds)
{
	usleep(1000L * miliseconds);
}
#endif

int main(int argc __UNUSED_PARAM, char *argv[] __UNUSED_PARAM) {
  NmeaMallocedBuffer buf;
  NmeaGenerator *gen;
  NmeaInfo info;
  size_t i;

  memset(&buf, 0, sizeof(buf));
  nmeaInfoClear(&info);
  nmeaTimeSet(&info.utc, &info.present, NULL);

  nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_PDOP);
  nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_HDOP);
  nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_VDOP);
  nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_ELV);

  gen = nmeaGeneratorCreate(NMEALIB_GENERATOR_ROTATE, &info);
  if (!gen) {
    return -1;
  }

  for (i = 0; i < 10000; i++) {
    size_t gen_sz = nmeaGeneratorGenerateFrom(&buf, &info, gen, //
        NMEALIB_SENTENCE_GPGGA //
        | NMEALIB_SENTENCE_GPGSA //
        | NMEALIB_SENTENCE_GPGSV //
        | NMEALIB_SENTENCE_GPRMC //
        | NMEALIB_SENTENCE_GPVTG);
    if (gen_sz) {
      printf("%s\n", buf.buffer);
      my_sleep(500);
    }
  }

  nmeaGeneratorDestroy(gen);

  free(buf.buffer);
  buf.buffer = NULL;
  buf.bufferSize = 0;

  return 0;
}
