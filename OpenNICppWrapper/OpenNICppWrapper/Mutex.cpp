#include "Mutex.h"


namespace xne {
	Mutex::Mutex()
	{
		XnStatus rc = ::xnOSCreateMutex(&handle_);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}
	}

	Mutex::Mutex(const XnChar* cpMutexName)
	{
		XnStatus rc = ::xnOSCreateNamedMutex(&handle_, cpMutexName);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}
	}

	Mutex::~Mutex()
	{
		::xnOSCloseMutex(&handle_);
	}

	void Mutex::Lock(XnUInt32 nMilliseconds /*= XN_WAIT_INFINITE*/)
	{
		XnStatus rc = ::xnOSLockMutex(&handle_, nMilliseconds);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		isLocked_ = true;
	}

	void Mutex::Unlock()
	{
		XnStatus rc = ::xnOSUnLockMutex(&handle_);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		isLocked_ = false;
	}
}
