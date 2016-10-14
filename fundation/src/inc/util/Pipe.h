#ifndef GUARD_Pipe_h__
#define GUARD_Pipe_h__
#include "../../inc/dep.h"
#include "../core/Types.h"
#include "./Semaphore.h"
class Pipe
	/// This class implements an anonymous pipe.
	///
	/// Pipes are a common method of inter-process communication -
	/// on Unix, pipes are the oldest form of IPC.
	///
	/// A pipe is a half-duplex communication channel, which means
	/// that data only flows in one direction.
	/// Pipes have a read-end and a write-end. One process writes to
	/// the pipe and another process reads the data written by
	/// its peer. 
	/// Read and write operations are always synchronous. A read will
	/// block until data is available and a write will block until
	/// the reader reads the data.
	///
	/// The sendBytes() and readBytes() methods of Pipe are usually
	/// used through a PipeOutputStream or PipeInputStream and are
	/// not called directly.
	///
	/// Pipe objects have value semantics; the actual work is delegated
	/// to a reference-counted PipeImpl object.
{
public:
#if BC_TARGET == BC_TARGET_LINUX
#elif BC_TARGET == BC_TARGET_WIN
	typedef HANDLE ID;
#endif
	enum CloseMode /// used by close()
	{
		CLOSE_READ = 0x01, /// Close reading end of pipe.
		CLOSE_WRITE = 0x02, /// Close writing end of pipe.
		CLOSE_BOTH = 0x03  /// Close both ends of pipe.
	};

	Pipe();
	/// Creates the Pipe.
	///
	/// Throws a CreateFileException if the pipe cannot be
	/// created.

	~Pipe();
	/// Releases the Pipe's PipeImpl and assigns another one.

	bool writeBytes(const void* buffer, unsigned int buffSize, unsigned int* written);
	/// Sends the contents of the given buffer through
	/// the pipe. Blocks until the receiver is ready
	/// to read the data.
	///
	/// Returns the number of bytes sent.
	///
	/// Throws a WriteFileException if the data cannot be written.

	bool readBytes(void* buffer, unsigned int buffSize, unsigned int* read);
	/// Receives data from the pipe and stores it
	/// in buffer. Up to length bytes are received.
	/// Blocks until data becomes available.
	///
	/// Returns the number of bytes received, or 0
	/// if the pipe has been closed.
	///
	/// Throws a ReadFileException if nothing can be read.

	ID getReadHandle() const;
	/// Returns the read handle or file descriptor
	/// for the Pipe. For internal use only.

	ID getWriteHandle() const;
	/// Returns the write handle or file descriptor
	/// for the Pipe. For internal use only.

	void close(CloseMode mode = CLOSE_BOTH);
	/// Depending on the argument, closes either the
	/// reading end, the writing end, or both ends
	/// of the Pipe.
private:
	ID _readHandle;
	ID _writeHandle;
};
class NamedPipe
{
#if BC_TARGET == BC_TARGET_LINUX
#elif BC_TARGET == BC_TARGET_WIN
	typedef HANDLE ID;
#endif
public:
	//size = 0 means open
	bool create(char* name);
	void destroy();
	~NamedPipe();
private:
	ID mem_id;
	void* p_map;
	Semaphore sem;
};
#endif // GUARD_Pipe_h__
