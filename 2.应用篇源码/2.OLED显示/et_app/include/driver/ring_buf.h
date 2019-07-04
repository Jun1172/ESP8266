#ifndef _RING_BUF_H_
#define _RING_BUF_H_
#include "et_types.h"

typedef struct 
{
	et_uint8 * p_o;				/**< Original pointer */
	et_uint8 * volatile p_r;		/**< Read pointer */
	et_uint8 * volatile p_w;		/**< Write pointer */
	volatile et_int32 fill_cnt;	/**< Number of filled slots */
	et_int32    size;				/**< Buffer size */
}ring_buf_t;

et_int16  ring_buf_init(ring_buf_t *r, et_uint8* buf, et_int32 size);
et_int16  ring_buf_put(ring_buf_t *r, et_uint8 c);
et_int16  ring_buf_get(ring_buf_t *r, et_uint8* c,et_int32 length);
#endif