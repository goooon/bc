#ifndef __BINARY_FORMATER_H__
#define __BINARY_FORMATER_H__

#include "./fundation.h"

#ifdef __cplusplus
extern "C" {
#endif

void *bf_create(u8 *stream, u32 size);
void *bf_create_encoder(void);
void *bf_create_decoder(u8 *stream, u32 size);
void bf_destroy(void *h);

/*
 * reset encode or decode current location
 */
void bf_reset(void *h, u32 index);

s32 bf_put_u8(void *h, u8 v);
s32 bf_put_u16(void *h, u16 v);
s32 bf_put_u24(void *h, u32 v);
s32 bf_put_u32(void *h, u32 v);
s32 bf_put_u64(void *h, u64 v);
s32 bf_put_bytes(void *h, u8 *data, u32 len);
s32 bf_put_string(void *h, const char *s);

s32 bf_read_u8(void *h, u8 *v);
s32 bf_read_u16(void *h, u16 *v);
s32 bf_read_u24(void *h, u32 *v);
s32 bf_read_u32(void *h, u32 *v);
s32 bf_read_u64(void *h, u64 *v);
s32 bf_read_bytes(void *h, u8 **v, u32 *len);
s32 bf_read_string(void *h, char **v);

#ifdef __cplusplus
}
#endif

#endif // __BINARY_FORMATER_H__

