#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#define __WINOS__ 1
#include <windows.h>
#else
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>  
#endif

#include "../inc/bcp_serial.h"

typedef struct timeout_s {
	int maxtime;
	clock_t currtime;
	int state;
} timeout_t;

static void timeout_init(timeout_t *t, int time)
{
	t->maxtime = time;
	t->state = 0;
}

static void timeout_start(timeout_t *t)
{
	t->currtime = clock();
	t->state = 1;
}

static int is_expired(timeout_t *t)
{
	return (((clock() - t->currtime) / (double) (CLOCKS_PER_SEC / 1000)) >= t->maxtime);
}

static int timeout_expired(timeout_t *t)
{
	if (t->state && is_expired(t)) {
		t->state = 0;
		return 1;
	} else {
		return 0;
	}
}

typedef struct bcp_serial_s
{
#if defined(__WINOS__)
	HANDLE hdl;
	DCB conf;
#else
	int hdl;
	struct termios oldtio;
	struct termios newtio;
#endif
	timeout_t to;
} bcp_serial_t;

#if defined(__WINOS__)
void *bcp_serial_open(const char *port, int baud, char bits, parity parity, char stopbit) 
{
	bcp_serial_t *s;
	char tmp[256];
	COMMTIMEOUTS timeouts = {1000U, 1000U, 
		1000U, 1000U, 1000U };

	s = (bcp_serial_t*)malloc(sizeof(*s));
	if (!s) {
		return NULL;
	} else {
		memset(s, 0, sizeof(*s));
	}

	sprintf(tmp, "\\\\.\\%s", port);
	s->hdl = CreateFileA(tmp,
				  GENERIC_READ | GENERIC_WRITE, 
				  0,
				  NULL,
				  OPEN_EXISTING,
				  0,
				  NULL);

	if (s->hdl == INVALID_HANDLE_VALUE) {
        printf("E:Error bcp_serial_open %s Port error(%d)\n", tmp,GetLastError());
        goto __failed;
    }

	SetupComm(s->hdl, 4096, 4096);

	if (!GetCommState(s->hdl, &s->conf)) {
		printf("E:Error bcp_serial_open GetCommState %s Port\n", tmp);
		goto __failed;
	}

#if 0
	// Reset settings
	s->conf.fOutxCtsFlow = FALSE;
	s->conf.fOutxDsrFlow = FALSE;
	s->conf.fOutX = FALSE;
	s->conf.fInX = FALSE;
	s->conf.fNull = 0;
	s->conf.fAbortOnError = 0;
	//s->conf.fRtsControl=RTS_CONTROL_TOGGLE;
#endif

	// Set baud rate
	switch (baud)
	{
		case 9600  : s->conf.BaudRate = CBR_9600  ; break;
		case 19200 : s->conf.BaudRate = CBR_19200 ; break;
		case 38400 : s->conf.BaudRate = CBR_38400 ; break;
		case 57600 : s->conf.BaudRate = CBR_57600 ; break;
		case 115200: s->conf.BaudRate = CBR_115200; break;
		default	   : s->conf.BaudRate = CBR_9600;
	}

	// Set byte size
    switch (bits)
    {
		case 5	: s->conf.ByteSize = 5; break;
    	case 6	: s->conf.ByteSize = 6; break;
    	case 7	: s->conf.ByteSize = 7; break;
    	case 8	: s->conf.ByteSize = 8; break;
    	default : s->conf.ByteSize = 8; break;
    }

	// Set parity
	switch (parity)
	{
		case P_NONE	 : s->conf.Parity = NOPARITY;   break;
		case P_EVEN: s->conf.Parity = EVENPARITY; break;
		case P_ODD : s->conf.Parity = ODDPARITY;  break;
		default	 : s->conf.Parity = NOPARITY;
	}

	// Set stop bit
	switch (stopbit)
	{
		case 1 : s->conf.StopBits = ONESTOPBIT;  break;
		case 2 : s->conf.StopBits = TWOSTOPBITS; break;
		default: s->conf.StopBits = ONESTOPBIT;
	}

	// Configure serial port
	if (!SetCommTimeouts(s->hdl, &timeouts)) {
		printf("Error SetCommTimeouts %s Port\n", tmp);
        goto __failed;
	}

	if (!SetCommMask(s->hdl, EV_RXCHAR)) {
		printf("Error SetCommMask(EV_RXCHAR) %s Port\n", tmp);
		goto __failed;
	}

	if (!SetCommState(s->hdl, &s->conf)) {
		printf("Error SetCommState %s Port\n", tmp);
		goto __failed;
	}

	// Clean errors and rx/tx buffer
	PurgeComm(s->hdl, PURGE_RXABORT | PURGE_TXCLEAR 
		| PURGE_TXABORT | PURGE_TXCLEAR);

	printf("%s | BaudRate: %d | Bits: %d | Parity: %d | StopBits: %d\n", 
		tmp, baud, bits, parity, stopbit);

	return s;

__failed:
	bcp_serial_close(s);
	return NULL;
}

void bcp_serial_close(void *hdl) 
{
	bcp_serial_t *s;
	
	if (!hdl) {
		return;
	} else {
		s = (bcp_serial_t*)hdl;
	}

	if (s->hdl && s->hdl != INVALID_HANDLE_VALUE) {
		CloseHandle(s->hdl);
		s->hdl = 0;
	}

	free(s);
}

int bcp_serial_write(void *hdl, const char *buffer, int len) 
{
	DWORD r = 0;
	bcp_serial_t *s;
	const uint8_t *p = (uint8_t*)buffer;

	if (!hdl || !p) {
		return -1;
	} else {
		s = (bcp_serial_t*)hdl;
	}

	while (len > 0 && r >= 0) {
		if (WriteFile(s->hdl, p, len, &r, NULL) && r > 0) {
			len -= r;
			p += r;
		}
	}

	return (const char*)p - buffer;
}

int bcp_serial_read(void *hdl, char *buffer, int len, int timout)
{
	BOOL ret;
	DWORD r = 0;
	bcp_serial_t *s;
	int leftlen = len;
	char *p = buffer;

	if (!hdl || !p) {
		return -1;
	} else {
		s = (bcp_serial_t*)hdl;
	}

	memset(buffer, 0, len);

	timeout_init(&s->to, timout);
	timeout_start(&s->to);

	while (leftlen > 0 && !timeout_expired(&s->to)) {
		ret = ReadFile(s->hdl, p, (DWORD)leftlen, &r, NULL);
		if (ret && r > 0) {
			leftlen -= r;
			p += r;
			timeout_start(&s->to);
		}
	}

	return (const char*)p - buffer;
}

#else // !defined(__WINOS__)

void *bcp_serial_open(const char *port, int baud, char bits, parity parity, char stopbit) 
{
	bcp_serial_t *s;
	char tmp[256];

	s = (bcp_serial_t*)malloc(sizeof(*s));
	if (!s) {
		return NULL;
	} else {
		memset(s, 0, sizeof(*s));
	}

	sprintf(tmp, "%s", port);
	s->hdl = open(tmp, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (s->hdl < 0) {
        printf("Error Opening %s Port\n", tmp);
		goto __failed;
	}

	// Get terminal parameters
	tcgetattr(s->hdl, &s->newtio);
	tcgetattr(s->hdl, &s->oldtio);

	// Flushes data received but not read
	ioctl(s->hdl, TCIFLUSH);

	// Set baud rate (in and out)
	switch (baud) {
		case 9600	: cfsetspeed(&s->newtio, B9600)	 ; break;
		case 19200	: cfsetspeed(&s->newtio, B19200) ; break;
		case 38400	: cfsetspeed(&s->newtio, B38400) ; break;
		case 57600	: cfsetspeed(&s->newtio, B57600) ; break;
		case 115200	: cfsetspeed(&s->newtio, B115200); break;
		default 	: cfsetspeed(&s->newtio, B9600)  ; break;
	}

	// Set byte size
	s->newtio.c_cflag &= ~CSIZE;	

	switch (bits) {
		case 5: s->newtio.c_cflag |= CS5; break;
		case 6: s->newtio.c_cflag |= CS6; break;
		case 7: s->newtio.c_cflag |= CS7; break;
		case 8: s->newtio.c_cflag |= CS8; break;
		default: s->newtio.c_cflag |= CS8; break;
	}

	// Set parity
	switch (parity) {
		case P_NONE:
			s->newtio.c_cflag &=~ PARENB;	// Disable parity
			break;
		case P_EVEN:
			s->newtio.c_cflag |= PARENB;		// Enable parity
			s->newtio.c_cflag &= ~PARODD;	// Disable odd parity
			break;
		case P_ODD:
			s->newtio.c_cflag |= PARENB;		// Enable parity
			s->newtio.c_cflag |= PARODD;		// Enable odd parity
			break;
		default:
			s->newtio.c_cflag &=~ PARENB;	// Disable parity
			break;
	}
	
	// Set stop bit
	switch (stopbit) {
		case 1: s->newtio.c_cflag &=~ CSTOPB; break;	// Disable 2 stop bits
		case 2: s->newtio.c_cflag |= CSTOPB; break;	// Enable 2 stop bits
		default: s->newtio.c_cflag &=~ CSTOPB; break;
	}

	// Enable receiver (CREAD) and ignore modem control lines (CLOCAL)
	s->newtio.c_cflag |= (CREAD | CLOCAL); 
	
	// Disable, canonical mode (ICANON = 0), echo input character (ECHO) and signal generation (ISIG)
	s->newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
	
	// Disable input parity checking (INPCK)
	s->newtio.c_iflag &= ~INPCK; 		  

	// Disable XON/XOFF flow control on output and input (IXON/IXOFF), framing and parity errors (IGNPAR), and disable CR to NL translation
	s->newtio.c_iflag &= ~(IXON | IXOFF | IXANY | IGNPAR | ICRNL);

	// Disable implementation-defined output processing (OPOST)
	s->newtio.c_oflag &=~ OPOST;	

	// Set terminal parameters
	tcsetattr(s->hdl, TCSAFLUSH, &s->newtio);

	// Display settings
	printf("%s | BaudRate: %d | Bits: %d | Parity: %d | StopBits: %d\n", tmp, baud, bits, parity, stopbit);

	return s;

__failed:
	bcp_serial_close(s);
	return NULL;
}
void bcp_serial_close(void *hdl) 
{
	bcp_serial_t *s;

	if (!hdl) {
		return;
	} else {
		s = (bcp_serial_t*)hdl;
	}

	tcsetattr(s->hdl, TCSANOW, &s->oldtio);

	if (s->hdl > 0) {
		close(s->hdl);
		s->hdl = -1;
	}

	free(s);
}

int bcp_serial_write(void *hdl, const char* buffer, int len) 
{
	bcp_serial_t *s;
	ssize_t r = 0;
	const uint8_t *p = (const uint8_t*)buffer;

	if (!hdl || !p) {
		return -1;
	} else {
		s = (bcp_serial_t*)hdl;
	}

	while (len > 0 && r >= 0) {
		if ((r = write(s->hdl, p, len)) > 0) {
			len -= r;
			p += r;
		}
	}

	return (const char*)p - buffer;
}

static int serial_wait(int fd, int timeout)
{
	int ret;
	fd_set rfds;
    struct timeval tv;

	if (timeout >= 0) {
	    tv.tv_sec = timeout / 1000;
	    tv.tv_usec = (timeout % 1000) * 1000;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		if ((ret = select(fd + 1, &rfds, NULL, NULL, &tv)) < 0 || !ret)
			return -1;
	}

	return 0;
}

int bcp_serial_read(void *hdl, char *buffer, int len, int timeout)
{
	bcp_serial_t *s;
	ssize_t r = 0;
	int leftlen = len;
	char *p = buffer;

	if (!hdl || !p) {
		return -1;
	} else {
		s = (bcp_serial_t*)hdl;
	}

	memset(p, 0, len);

	while (leftlen > 0 && 
		serial_wait(s->hdl, timeout) >= 0) {
		r = read(s->hdl, p, leftlen);
		if (r > 0) {
			leftlen -= r;
			p += r;
		} else if (r < 0 && r != EAGAIN) {
			break;
		}
	}

	if (r < 0 && (p - buffer) == 0) {
		return r;
	} else {
		return p - buffer;
	}
}
#endif

