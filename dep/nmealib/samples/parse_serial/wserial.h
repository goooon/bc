#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

#if defined (_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define __WINOS__ 1
#endif

#if defined(__WINOS__)
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/ioctl.h>
	#include <termios.h>
	#include <string.h>
	#include <errno.h>
#endif

#define BUFFER_SIZE 256

enum parity
{
	NO,
	EVEN,
	ODD
};

class serial
{
	private:
		
#if defined(__WINOS__)
		HANDLE m_fd;
		DCB m_conf;
#else
		int	m_fd;
		struct termios m_oldtio;
		struct termios m_newtio;
#endif

	public:
		char open(char *port, int baud, char bits, parity parity, char stopbit);
		void close(void);
		char write(char *buffer, int length);
		int read(char *buffer, int length, int timeout /* unit ms */);
};

#endif
