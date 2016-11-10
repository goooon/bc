#ifndef __BCP_NMEA_H__
#define __BCP_NMEA_H__

typedef struct nmea_time_s {
  unsigned int year;
  unsigned int mon;
  unsigned int day;
  unsigned int hour;
  unsigned int min;
  unsigned int sec;
  unsigned int milisec;
} nmea_time_t;

/**
 * position data in decimal degrees or radians
 */
typedef struct nmea_pos_s {
  double lat;
  double lon;
} nmea_pos_t;

/**
 * information about satellite
 */
typedef struct nmea_satellite_s {
  unsigned int prn;       /**< satellite PRN number             - [1, inf) */
  int          elevation; /**< elevation, in degrees            - [0,  90] */
  unsigned int azimuth;   /**< azimuth, degrees from true north - [0, 359] */
  unsigned int snr;       /**< signal-to-Noise-Ratio            - [0,  99] */
} nmea_satellite_t;

#define NMEA_MAX_SATELLITES (72u)

/**
 * information about all tracked satellites
 */
typedef struct nmea_satellites_s {
  unsigned int  use_count;
  unsigned int  use[NMEA_MAX_SATELLITES];
  unsigned int  view_count;
  nmea_satellite_t view[NMEA_MAX_SATELLITES];
} nmea_satellites_t;

/**
 * information about progress on non-atomic sentences
 */
typedef struct nmea_progress_s {
  bool gpgsv_inprogress; /**< true when gpgsv is in progress */
} nmea_progress_t;

/**
 * GPS information from all supported sentences
 */
typedef struct nmea_info_s {
  unsigned int   present;    /**< Bit-mask specifying which fields are present                    */
  unsigned int   smask;      /**< Bit-mask specifying from which sentences data has been obtained */
  nmea_time_t    localtime;	 /**< local time from GPZDA                                           */
  nmea_time_t    utc;        /**< UTC of the position data                                        */
  int     		 sig;        /**< Signal quality, see NMEALIB_SIG_* signals                       */
  int	         fix;        /**< Operating mode, see NMEALIB_FIX_* fixes                         */
  double         pdop;       /**< Position Dilution Of Precision                                  */
  double         hdop;       /**< Horizontal Dilution Of Precision                                */
  double         vdop;       /**< Vertical Dilution Of Precision                                  */
  double         latitude;   /**< Latitude,  in NDEG: +/-[degree][min].[sec/60]                   */
  double         longitude;  /**< Longitude, in NDEG: +/-[degree][min].[sec/60]                   */
  double         elevation;  /**< Elevation above/below mean sea level (geoid), in meters         */
  double         height;     /**< Height of geoid (elevation) above WGS84 ellipsoid, in meters    */
  double         speed;      /**< Speed over the ground in kph                                    */
  double         track;      /**< Track angle in degrees true north                               */
  double         mtrack;     /**< Magnetic Track angle in degrees true north                      */
  double         magvar;     /**< Magnetic variation in degrees                                   */
  double         dgpsAge;    /**< Time since last DGPS update, in seconds                         */
  unsigned int   dgpsSid;    /**< DGPS station ID number                                          */
  nmea_satellites_t satellites; /**< Satellites information                                          */
  nmea_progress_t  progress;   /**< Progress information                                            */
  bool           metric;     /**< When true then units are metric                                 */
} bcp_nmea_info_t;

typedef struct bcp_nmea_s
{
	bcp_nmea_info_t info;
	void *parser;
} bcp_nmea_t;

void *bcp_nmea_create(void(*fun_trace)(const char*, size_t), void(*fun_error)(const char*, size_t));
void bcp_nmea_destroy(void *hdl);

int bcp_nmea_parse(void *hdl, const char *buf, int len);
bcp_nmea_info_t *bcp_nmea_info(void *hdl);

const char *bcp_nmea_sig_to_string(int sig);
const char *bcp_nmea_fix_to_string(int fix);

#endif // __BCP_NMEA_H__
