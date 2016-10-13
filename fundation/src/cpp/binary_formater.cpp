#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/binary_formater.h"

/*
 * impl data serialize and unserialize
 * encoded big-endian for u8, u16, u24, u32, u64
 * encoded utf8 for string
 *
 * binary stream format, such as:
 *------------------------------
 *|				u8			   |
 *------------------------------
 *|				u16			   |
 *------------------------------
 *|				u32			   |
 *------------------------------
 *|				u64			   |
 *------------------------------
 *|			bytes length(u32)  |
 *------------------------------
 *|			bytes data		   |
 *------------------------------
 *|	  utf8 string length(u32)  |
 *------------------------------
 *|	   utf8 string data		   |
 *------------------------------
*/

#define MAX_DEF_BUF_SIZE (8 * 1024)

#define PUT_BYTES(h, v, bytes) \
	do { \
		int i; \
		bf_t *f = (bf_t*)h; \
		for (i = bytes - 1; i >= 0; i--) { \
			f->stream[f->index++] = (v >> (i *  8)) & 0xff; \
		} \
	} while(0)

#define READ_BYTES(h, pv, vtype, bytes) \
	do { \
		int i; \
		bf_t *f = (bf_t*)h; \
		*pv = 0; \
		for (i = bytes - 1; i >= 0; i--) { \
			*pv |= ((vtype)f->stream[f->index++]) << (i * 8); \
		} \
	} while (0)

typedef struct bf_s
{
	u8 new_buf; /* create stream buffer */
	int index; /* next index of stream */
	u8 *stream; /* serialize data */
	u32 size; /* stream size */
} bf_t;

void *bf_create(u8 *stream, u32 size)
{
	bf_t *f;

	f = (bf_t*)malloc(sizeof(*f));
	if (!f) {
		return NULL;
	}

	f->new_buf = 0;
	f->index = 0;
	f->stream = stream;
	f->size = size;

	return f;
}

void *bf_create_encoder(void)
{
	return bf_create(NULL, 0);
}

void *bf_create_decoder(u8 *stream, u32 size)
{
	return bf_create(stream, size);
}

void bf_destroy(void *h)
{
	bf_t *bf = (bf_t*)h;

	if (!bf) {
		return;
	}

	if (bf->new_buf) {
		free(bf->stream);
	}

	free(bf);
}

void bf_reset(void *h, u32 index)
{
	bf_t *bf = (bf_t*)h;

	if (!bf) {
		return;
	}

	bf->index = index;
}

static s32 check_encode_buf(void *h, u32 req_size)
{
	bf_t *f = (bf_t*)h;
	u8 *s;
	u32 new_size;

	if (!f) {
		return -1;
	}
	
	new_size = f->index + req_size;
	if (new_size > f->size) {
		if (!f->size) {
			/* first put data, malloc max buf */
			if (new_size < MAX_DEF_BUF_SIZE) {
				new_size = MAX_DEF_BUF_SIZE;
			}
		}
		s = (u8*)malloc(new_size);
		if (!s) {
			return -1;
		}
		if (f->size > 0) {
			memcpy(s, f->stream, f->size);
			/* malloc myself can be free */
			if (f->new_buf) {
				free(f->stream);
			}
		}
		f->new_buf = 1;
		f->size = new_size;
		f->stream = s;
	}

	return 0;
}

static s32 check_decode_buf(void *h, u32 req_size)
{
	bf_t *f = (bf_t*)h;

	if (!f) {
		return -1;
	}

	if (f->index + req_size > f->size) {
		return -1;
	} else {
		return 0;
	}
}

s32 bf_put_u8(void *h, u8 v)
{
	if (check_encode_buf(h, 1) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, v, 1);
		return 0;
	}
}

s32 bf_put_u16(void *h, u16 v)
{
	if (check_encode_buf(h, 2) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, v, 2);
		return 0;
	}

}

s32 bf_put_u24(void *h, u32 v)
{
	if (check_encode_buf(h, 3) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, v, 3);
		return 0;
	}
}

s32 bf_put_u32(void *h, u32 v)
{
	if (check_encode_buf(h, 4) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, v, 4);
		return 0;
	}
}

s32 bf_put_u64(void *h, u64 v)
{
	if (check_encode_buf(h, 8) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, v, 8);
		return 0;
	}
}

s32 bf_put_bytes(void *h, u8 *data, u32 len)
{
	bf_t *f = (bf_t*)h;
	
	if (!f || !data || !len) {
		return -1;
	}

	if (check_encode_buf(h, len + 4) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, len, 4);
		memcpy(&f->stream[f->index], data, len);
		f->index += len;
		return 0;
	}
}

/* TODO: encode to utf8 */
s32 bf_put_string(void *h, const char *s)
{
	bf_t *f = (bf_t*)h;
	u32 len;
	
	if (!f || !s) {
		return -1;
	}

	len = (u32)strlen(s);
	if (!len) {
		return -1;
	}

	if (check_encode_buf(h, len + 4) < 0) {
		return -1;
	} else {
		PUT_BYTES(h, len, 4);
		memcpy(&f->stream[f->index], s, len);
		f->index += len;
		return 0;
	}
}

s32 bf_read_u8(void *h, u8 *v)
{
	if (!v || check_decode_buf(h, 1) < 0) {
		return -1;
	} else {
		READ_BYTES(h, v, u8, 1);
		return 0;
	}
}

s32 bf_read_u16(void *h, u16 *v)
{
	if (!v || check_decode_buf(h, 2) < 0) {
		return -1;
	} else {
		READ_BYTES(h, v, u16, 2);
		return 0;
	}
}

s32 bf_read_u24(void *h, u32 *v)
{
	if (!v || check_decode_buf(h, 3) < 0) {
		return -1;
	} else {
		READ_BYTES(h, v, u32, 3);
		return 0;
	}
}

s32 bf_read_u32(void *h, u32 *v)
{
	if (!v || check_decode_buf(h, 4) < 0) {
		return -1;
	} else {
		READ_BYTES(h, v, u32, 4);
		return 0;
	}
}

s32 bf_read_u64(void *h, u64 *v)
{
	if (!v || check_decode_buf(h, 8) < 0) {
		return -1;
	} else {
		READ_BYTES(h, v, u64, 8);
		return 0;
	}
}

s32 bf_read_bytes(void *h, u8 **v, u32 *len)
{
	bf_t *f = (bf_t*)h;
	u8 *data;
	u32 bytes = 0;

	if (!v || !len || check_decode_buf(h, 4) < 0) {
		return -1;
	}
	
	/* read bytes length */
	READ_BYTES(h, (&bytes), u32, 4);
	if (!bytes || check_decode_buf(h, bytes) < 0) {
		return -1;
	}

	data = (u8*)malloc(bytes);
	if (!data) {
		return -1;
	}

	memcpy(data, &f->stream[f->index], bytes);
	f->index += bytes;

	*v = data;
	*len = bytes;

	return 0;
}

s32 bf_read_string(void *h, char **v)
{
	bf_t *f = (bf_t*)h;
	u8 *data;
	u32 bytes = 0;

	if (!v || check_decode_buf(h, 4) < 0) {
		return -1;
	}
	
	/* read bytes length */
	READ_BYTES(h, (&bytes), u32, 4);
	if (!bytes || check_decode_buf(h, bytes) < 0) {
		return -1;
	}

	data = (u8*)malloc(bytes + 1);
	if (!data) {
		return -1;
	}

	memcpy(data, &f->stream[f->index], bytes);
	f->index += bytes;
	data[bytes] = '\0';

	*v = (char*)data;

	return 0;
}

