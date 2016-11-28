#ifndef __ANDROID_VICP_H__
#define __ANDROID_VICP_H__

#include <stdint.h>

int android_vicp_init(void);
void android_vicp_notify(int32_t app_id,
	uint8_t *buf, int32_t len);

#endif // __ANDROID_VICP_H__