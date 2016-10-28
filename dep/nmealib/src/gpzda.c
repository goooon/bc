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

#include <nmealib/gpzda.h>

#include <nmealib/context.h>
#include <nmealib/sentence.h>
#include <nmealib/validate.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool nmeaGPZDAParse(const char *s, const size_t sz, NmeaGPZDA *pack) {
  size_t tokenCount;
  char timeBuf[32] = {0,};

  if (!pack) {
    return false;
  }

  memset(pack, 0, sizeof(*pack));

  if (!s //
      || !sz) {
    return false;
  }

  nmeaContextTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */

  /* parse */
  tokenCount = nmeaScanf(s, sz, //
      "$" NMEALIB_GPZDA_PREFIX ",%16s" //
      ",%u,%u,%u" //
      ",%d,%u*",//
      timeBuf, 
      &pack->utc.day, &pack->utc.mon, &pack->utc.year,
      &pack->local_hours, &pack->local_minutes
      );

  /* see that there are enough tokens */
  if (tokenCount != 6) {
    nmeaContextError(NMEALIB_GPZDA_PREFIX " parse error: need 6 tokens, got %lu in '%s'", (long unsigned) tokenCount,
      s);
    goto err;
  }

  if ((pack->utc.day == 0)
      || (pack->utc.mon == 0)
      || (pack->utc.year == 0)) {
    nmeaContextError(NMEALIB_GPZDA_PREFIX " parse error: day = mon = year = 0 in '%s'",
      s);
    goto err;
  }

  /* check data */
  if (!nmeaValidateDate(&pack->utc, NMEALIB_GPZDA_PREFIX, s)) {
    goto err;
  } else {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LOCALDATE);
  }

  if (!*timeBuf || !nmeaTimeParseTime(timeBuf, &pack->utc) //
      || !nmeaValidateTime(&pack->utc, NMEALIB_GPZDA_PREFIX, s)) {
    goto err;
  } else {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LOCALTIME);
  }

  if (pack->local_hours < -13 || pack->local_hours > 13 
  	|| pack->local_minutes > 59) {
    nmeaContextError(NMEALIB_GPZDA_PREFIX " parse error: local_hours = %d, local_mintues = %u in '%s'",
      pack->local_hours, pack->local_minutes, s);
	goto err;
  } else {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LOCALZONE);
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  return false;
}

void nmeaGPZDAToInfo(const NmeaGPZDA *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LOCALDATE)) {
    info->localtime.hour = pack->utc.hour;
    info->localtime.min = pack->utc.min;
    info->localtime.sec = pack->utc.sec;
    info->localtime.hsec = pack->utc.hsec;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LOCALDATE);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LOCALTIME)) {
    info->localtime.day = pack->utc.day;
    info->localtime.mon = pack->utc.mon;
    info->localtime.year = pack->utc.year;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LOCALTIME);
  }

  /* TODO: local zone */

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SMASK);
  info->smask |= NMEALIB_SENTENCE_GPZDA;
}

void nmeaGPZDAFromInfo(const NmeaInfo *info, NmeaGPZDA *pack) {
  size_t inViewCount;
  size_t sentences;

  if (!pack) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (!info) {
  	return;
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_UTCDATE)) {
    pack->utc.day = info->localtime.day;
    pack->utc.mon = info->localtime.mon;
    pack->utc.year = info->localtime.year;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_UTCDATE);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_UTCTIME)) {
    pack->utc.hour = info->localtime.hour;
    pack->utc.min = info->localtime.min;
    pack->utc.sec = info->localtime.sec;
    pack->utc.hsec = info->localtime.hsec;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_UTCTIME);
  }

  /* TODO: local zone */
  
}

size_t nmeaGPZDAGenerate(char *s, const size_t sz, const NmeaGPZDA *pack) {

#define dst       (&s[chars])
#define available ((sz <= (size_t) chars) ? 0 : (sz - (size_t) chars))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEALIB_GPZDA_PREFIX);
  
  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_UTCTIME)) {
    chars += snprintf(dst, available, //
        ",%02u%02u%02u.%02u", //
        pack->utc.hour, //
        pack->utc.min, //
        pack->utc.sec, //
        pack->utc.hsec);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_UTCDATE)) {
    chars += snprintf(dst, available, //
        ",%02u,%02u,%02u", //
        pack->utc.day, //
        pack->utc.mon, //
        pack->utc.year % 100);
  } else {
    chars += snprintf(dst, available, ",,,");
  }

  /* local zone hours minutes */
  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LOCALZONE)) {
    chars += snprintf(dst, available, //
        ",%02u,%02u", //
        pack->local_hours, //
        pack->local_minutes);
  } else {
    chars += snprintf(dst, available, ",00,00");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;

#undef available
#undef dst

}

