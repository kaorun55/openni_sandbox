#ifndef MUTEX_H_INCLUDE
#define MUTEX_H_INCLUDE

#include <XnOS.h>
#include <boost/utility.hpp>
#include "SyncLock.h"

namespace xne {
	class Mutex : public SyncObject, public boost::noncopyable
	{
	public:

		Mutex();
		Mutex(const XnChar* cpMutexName);
		~Mutex();

		void Lock(XnUInt32 nMilliseconds = XN_WAIT_INFINITE);
		void Unlock();

	private:

		XN_MUTEX_HANDLE handle_;
	};
}

#endif // #ifndef MUTEX_H_INCLUDE
