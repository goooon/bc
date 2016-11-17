#ifndef __BINARY_FORMATER_H__
#define __BINARY_FORMATER_H__

#include "./fundation.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BF_STACK_BUFF_SIZE 128

typedef struct bf_s
{
	u8 _s[BF_STACK_BUFF_SIZE]; /* stack buffer for small encode stream */
	u8 new_buf; /* create stream buffer */
	u32 index; /* next index of stream */
	u8 *stream; /* un/serialize data */
	u32 size; /* stream size */
} bf_t;

void bf_init(bf_t *f, u8 *stream, u32 size);

/*
 * init stack bf_t for encoder 
 */
int bf_init_e(bf_t *f, u32 size);

/* 
 * init stack bf_t for decoder
 */
void bf_init_d(bf_t *f, u8 *stream, u32 size);

void bf_uninit(bf_t *f, int free_stream);

/*
 * create heap bf
 */
bf_t *bf_create(u8 *stream, u32 size);
bf_t *bf_create_encoder(void);
bf_t *bf_create_decoder(u8 *stream, u32 size);
void bf_destroy_p(bf_t *f, int free_stream);
void bf_destroy(bf_t *f); /* force free new buf */

/*
 * reset encode or decode current location
 */
void bf_reset(bf_t *f, u32 index);

s32 bf_put_u8(bf_t *f, u8 v);
s32 bf_put_u16(bf_t *f, u16 v);
s32 bf_put_u24(bf_t *f, u32 v);
s32 bf_put_u32(bf_t *f, u32 v);
s32 bf_put_u64(bf_t *f, u64 v);
s32 bf_put_bytes_only(bf_t *f, u8 *data, u32 len);
s32 bf_put_bytes(bf_t *f, u8 *data, u32 len);
s32 bf_put_string_only(bf_t *f, const char *s);
s32 bf_put_string(bf_t *f, const char *s);

s32 bf_read_u8(bf_t *f, u8 *v);
s32 bf_read_u16(bf_t *f, u16 *v);
s32 bf_read_u24(bf_t *f, u32 *v);
s32 bf_read_u32(bf_t *f, u32 *v);
s32 bf_read_u64(bf_t *f, u64 *v);
s32 bf_read_bytes_only(bf_t *f, u8 **v, u32 len);
s32 bf_read_bytes(bf_t *f, u8 **v, u32 *len);

s32 bf_read_string_only(bf_t *f, char **v, u32 len);
s32 bf_read_string(bf_t *f, char **v);

u8 *bf_stream(bf_t *f); /* stream ptr */
u32 bf_size(bf_t *f); /* stream used buf size */

#ifdef __cplusplus
}
#endif

#endif // __BINARY_FORMATER_H__

