#include "./BCMessage.h"
#include "./PackageQueue.h"
bool BCPackage::storeForResend(u8* buf,u32 len)
{
	return PackageQueue::getInstance().in(buf, len);
}
