#include "../../inc/util/Pipe.h"

bool NamedPipe::create(char* name)
{
	return false;
}

void NamedPipe::destroy()
{

}

NamedPipe::~NamedPipe()
{

}

Pipe::Pipe()
{

}

Pipe::~Pipe()
{

}

bool Pipe::writeBytes(const void* buffer, unsigned int buffSize, unsigned int* written)
{
	return false;
}

bool Pipe::readBytes(void* buffer, unsigned int buffSize, unsigned int* read)
{
	return false;
}

Pipe::ID Pipe::getReadHandle() const
{
	return 0;
}

Pipe::ID Pipe::getWriteHandle() const
{
	return 0;
}
