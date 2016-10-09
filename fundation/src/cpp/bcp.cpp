#include <stdio.h>
#include "../inc/bcp_packet.h"
#include "../inc/bcp_comm.h"
#include "../inc/bcp.h"

void bcp_init(void)
{
	bcp_packet_init();
}

void bcp_uninit(void)
{
	bcp_packet_uninit();
}
