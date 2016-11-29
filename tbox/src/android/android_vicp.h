#ifndef __ANDROID_VICP_H__
#define __ANDROID_VICP_H__

#include <stdint.h>

/* start vicp service server */
int android_vicp_server_start(void);
/* notify vicp data to service client */
void android_vicp_notify(int32_t app_id,
	uint8_t *buf, int32_t len);

/* start vicp service client */
void android_vicp_client_start(void);

#endif // __ANDROID_VICP_H__