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

#define MAX_DEF_BUF_SIZE (2 * 1024 + 64)

#define PUT_BYTES(f, v, bytes) \
	do { \
		int i; \
		for (i = bytes - 1; i >= 0; i--) { \
			f->stream[f->index++] = (v >> (i *  8)) & 0xff; \
		} \
	} while(0)

#define READ_BYTES(f, pv, vtype, bytes) \
	do { \
		int i; \
		*pv = 0; \
		for (i = bytes - 1; i >= 0; i--) { \
			*pv |= ((vtype)f->stream[f->index++]) << (i * 8); \
		} \
	} while (0)

void bf_init(bf_t *f, u8 *stream, u32 size)
{
	f->new_buf = 0;
	f->index = 0;
	f->stream = stream;
	f->size = size;
}

void bf_init_d(bf_t *f, u8 *stream, u32 size)
{
	bf_init(f, stream, size);
}

int bf_init_e(bf_t *f, u32 size)
{
	u8 *stream;
	u8 new_buf;

	if (size < BF_STACK_BUFF_SIZE) {
		stream = &f->_s[0];
		size = BF_STACK_BUFF_SIZE;
		new_buf = 0;
	} else {
		stream = (u8*)malloc(size);
		if (!stream) {
			return -1;
		} else {
			memset(stream, 0, size);
		}
		new_buf = 1;
	}

	bf_init(f, stream, size);
	f->new_buf = new_buf;

	return 0;
}

void bf_uninit(bf_t *f, int free_stream)
{
	f->index = 0;
	f->size = 0;
	if (f->new_buf && free_stream) {
		free(f->stream);
		f->stream = NULL;
		f->new_buf = 0;
	}
}

bf_t *bf_create(u8 *stream, u32 size)
{
	bf_t *f;

	f = (bf_t*)malloc(sizeof(*f));
	if (!f) {
		return NULL;
	}

	bf_init(f, stream, size);
	return f;
}

bf_t *bf_create_encoder(void)
{
	return bf_create(NULL, 0);
}

bf_t *bf_create_decoder(u8 *stream, u32 size)
{
	return bf_create(stream, size);
}

void bf_destroy_p(bf_t *f, int free_stream)
{
	if (!f) {
		return;
	}

	if (f->new_buf && free_stream) {
		free(f->stream);
	}

	free(f);
}

void bf_destroy(bf_t *f)
{
	bf_destroy_p(f, 1);
}

void bf_reset(bf_t *f, u32 index)
{
	if (!f) {
		return;
	}

	f->index = index;
}

static s32 check_encode_buf(bf_t *f, u32 req_size)
{
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

static s32 check_decode_buf(bf_t *f, u32 req_size)
{
	if (!f || !f->stream) {
		return -1;
	}

	if (f->index + req_size > f->size) {
		return -1;
	} else {
		return 0;
	}
}

s32 bf_put_u8(bf_t *f, u8 v)
{
	if (check_encode_buf(f, 1) < 0) {
		return -1;
	} else {
		PUT_BYTES(f, v, 1);
		return 0;
	}
}

s32 bf_put_u16(bf_t *f, u16 v)
{
	if (check_encode_buf(f, 2) < 0) {
		return -1;
	} else {
		PUT_BYTES(f, v, 2);
		return 0;
	}

}

s32 bf_put_u24(bf_t *f, u32 v)
{
	if (check_encode_buf(f, 3) < 0) {
		return -1;
	} else {
		PUT_BYTES(f, v, 3);
		return 0;
	}
}

s32 bf_put_u32(bf_t *f, u32 v)
{
	if (check_encode_buf(f, 4) < 0) {
		return -1;
	} else {
		PUT_BYTES(f, v, 4);
		return 0;
	}
}

s32 bf_put_u64(bf_t *f, u64 v)
{
	if (check_encode_buf(f, 8) < 0) {
		return -1;
	} else {
		PUT_BYTES(f, v, 8);
		return 0;
	}
}

s32 bf_put_bytes_only(bf_t *f, u8 *data, u32 len)
{
	if (!f) {
		return -1;
	}

	if (check_encode_buf(f, len) < 0) {
		return -1;
	} else {
		memcpy(&f->stream[f->index], data, len);
		f->index += len;
		return 0;
	}
}

s32 bf_put_bytes(bf_t *f, u8 *data, u32 len)
{
	if (bf_put_u32(f, len) < 0) {
		return -1;
	}

	return bf_put_bytes_only(f, data, len);
}

s32 bf_put_string_only(bf_t *f, const char *s)
{
	u32 len = 0;
	
	if (!f) {
		return -1;
	}

	if (s) {
		len = (u32)strlen(s);
	}

	if (check_encode_buf(f, len) < 0) {
		return -1;
	} else {
		memcpy(&f->stream[f->index], s, len);
		f->index += len;
		return 0;
	}
}

s32 bf_put_string(bf_t *f, const char *s)
{
	if (bf_put_u32(f, strlen(s)) < 0) {
		return -1;
	}

	return bf_put_string_only(f, s);
}

s32 bf_read_u8(bf_t *f, u8 *v)
{
	if (!v || check_decode_buf(f, 1) < 0) {
		return -1;
	} else {
		READ_BYTES(f, v, u8, 1);
		return 0;
	}
}

s32 bf_read_u16(bf_t *f, u16 *v)
{
	if (!v || check_decode_buf(f, 2) < 0) {
		return -1;
	} else {
		READ_BYTES(f, v, u16, 2);
		return 0;
	}
}

s32 bf_read_u24(bf_t *f, u32 *v)
{
	if (!v || check_decode_buf(f, 3) < 0) {
		return -1;
	} else {
		READ_BYTES(f, v, u32, 3);
		return 0;
	}
}

s32 bf_read_u32(bf_t *f, u32 *v)
{
	if (!v || check_decode_buf(f, 4) < 0) {
		return -1;
	} else {
		READ_BYTES(f, v, u32, 4);
		return 0;
	}
}

s32 bf_read_u64(bf_t *f, u64 *v)
{
	if (!v || check_decode_buf(f, 8) < 0) {
		return -1;
	} else {
		READ_BYTES(f, v, u64, 8);
		return 0;
	}
}

s32 bf_read_bytes_only(bf_t *f, u8 **v, u32 len)
{
	u8 *data = NULL;

	if (!v) {
		return -1;
	}

	if (check_decode_buf(f, len) < 0) {
		return -1;
	}

	if (len > 0) {
		data = (u8*)malloc(len);
		if (!data) {
			return -1;
		}
		memcpy(data, &f->stream[f->index], len);
		f->index += len;
	}

	*v = data;

	return 0;
}

s32 bf_read_bytes(bf_t *f, u8 **v, u32 *len)
{
	if (bf_read_u32(f, len) < 0) {
		return -1;
	}

	return bf_read_bytes_only(f, v, *len);
}

s32 bf_read_string_only(bf_t *f, char **v, int len)
{
	u8 *data = NULL;

	if (!v || !len) {
		return -1;
	}

	if (len > 0) {
		data = (u8*)malloc(len + 1);
		if (!data) {
			return -1;
		}
		memcpy(data, &f->stream[f->index], len);
		f->index += len;
		data[len] = '\0';
	}

	*v = (char*)data;

	return 0;
}

s32 bf_read_string(bf_t *f, char **v)
{
	u32 len;

	if (bf_read_u32(f, &len) < 0) {
		return -1;
	}

	return bf_read_string_only(f, v, len);
}

u8 *bf_stream(bf_t *f)
{
	if (f) {
		return f->stream;
	} else {
		return NULL;
	}
}

u32 bf_size(bf_t *f)
{
	if (f) {
		return f->index;
	} else {
		return 0;
	}
}

