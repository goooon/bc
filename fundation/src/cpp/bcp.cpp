#include <stdio.h>
#include "../inc/bcp_packet.h"
#include "../inc/bcp_comm.h"
#include "../inc/bcp_channel.h"
#include "../inc/vicp/bcp_vicp.h"
#include "../inc/bcp.h"

void bcp_init(void)
{
	bcp_packet_init();
	bcp_conn_init();
	bcp_channel_init();
	bcp_vicp_init();
}

void bcp_uninit(void)
{
	bcp_packet_uninit();
	bcp_conn_uninit();
	bcp_channel_uninit();
	bcp_vicp_uninit();
}
