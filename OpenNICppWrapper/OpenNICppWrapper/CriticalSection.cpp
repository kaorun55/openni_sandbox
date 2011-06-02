#include "CriticalSection.h"
#include <stdexcept>

namespace xne {
	CriticalSection::CriticalSection()
	{
		XnStatus rc = ::xnOSCreateCriticalSection(&handle_);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}
	}

	CriticalSection::~CriticalSection()
	{
		::xnOSCloseCriticalSection(&handle_);
	}

	void CriticalSection::Lock(XnUInt32 /*nMilliseconds = XN_WAIT_INFINITE*/)
	{
		XnStatus rc = ::xnOSEnterCriticalSection(&handle_);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		isLocked_ = true;
	}

	void CriticalSection::Unlock()
	{
		XnStatus rc = ::xnOSLeaveCriticalSection(&handle_);
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		isLocked_ = false;
	}
}
