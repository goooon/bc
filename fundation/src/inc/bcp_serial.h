#ifndef __BCP_SERIAL_H__
#define __BCP_SERIAL_H__

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

enum parity {
	P_NONE,
	P_EVEN,
	P_ODD
};

void *bcp_serial_open(const char *port, int baud, 
	char bits, parity parity, char stopbit) ;
void bcp_serial_close(void *hdl);
int bcp_serial_write(void *hdl, const char *buffer, int len);
int bcp_serial_read(void *hdl, char *buffer, 
	int len, int timout);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif // __BCP_SERIAL_H__