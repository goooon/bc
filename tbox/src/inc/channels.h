#ifndef __VICP_CHANNELS_H__
#define __VICP_CHANNELS_H__

#include "./dep.h"

void channels_init(void);
bcp_channel_t *channels_get(const char *name);
void channels_put(bcp_channel_t *c);
void channels_uninit(void);

#endif // __VICP_CHANNELS_H__
