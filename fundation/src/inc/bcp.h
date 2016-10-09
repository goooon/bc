#ifndef __BCP__H__
#define __BCP__H__

#include "./bcp_packet.h"
#include "./bcp_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

	void bcp_init(void);
	void bcp_uninit(void);

#ifdef __cplusplus
}
#endif

#endif // __BCP__H__
