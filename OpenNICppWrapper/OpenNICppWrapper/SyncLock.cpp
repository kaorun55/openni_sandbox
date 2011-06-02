#include "SyncLock.h"

namespace xne {
	SyncLock::SyncLock(SyncObject* sync, XnUInt32 nMilliseconds /*= XN_WAIT_INFINITE*/)
		: sync_(sync)
	{
		sync_->Lock(nMilliseconds);
	}

	SyncLock::~SyncLock(void)
	{
		if (sync_->IsLocked()) {
			sync_->Unlock();
		}
	}
}
