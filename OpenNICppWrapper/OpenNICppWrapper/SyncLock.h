#ifndef SYNCLOCK_H_INCLUDE
#define SYNCLOCK_H_INCLUDE

#include <XnOS.h>
#include <boost/utility.hpp>

namespace xne {
	class SyncObject : public boost::noncopyable
	{
	public:

		SyncObject() : isLocked_(false) {}
		virtual ~SyncObject(){}

		virtual void Lock(XnUInt32 nMilliseconds = XN_WAIT_INFINITE) = 0;
		virtual void Unlock() = 0;
		virtual bool IsLocked() { return isLocked_; }

	protected:

		bool isLocked_;
	};

	class SyncLock : public boost::noncopyable
	{
	public:
		SyncLock(SyncObject* sync, XnUInt32 nMilliseconds = XN_WAIT_INFINITE);
		~SyncLock(void);
		bool IsLocked() { return sync_->IsLocked(); }

	private:

		SyncObject* sync_;
	};
}

#endif // #ifndef SYNCLOCK_H_INCLUDE
