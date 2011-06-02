#ifndef CRITICALSECTION_H_INCLUDE
#define CRITICALSECTION_H_INCLUDE

#include <XnOS.h>
#include <boost/utility.hpp>
#include "SyncLock.h"

namespace xne {
	class CriticalSection : public SyncObject
	{
	public:
		CriticalSection();
		~CriticalSection();

		void Lock(XnUInt32 nMilliseconds = XN_WAIT_INFINITE);
		void Unlock();

	private:

		XN_CRITICAL_SECTION_HANDLE handle_;
	};
}

#endif // #ifndef CRITICALSECTION_H_INCLUDE
