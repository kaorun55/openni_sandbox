#pragma once 

#include <XnCppWrapper.h>

class SkeletonCallback
{
    friend class Skeleton;

public:

    virtual ~SkeletonCallback(){}

protected:

    static void XN_CALLBACK_TYPE CalibrationStart( xn::SkeletonCapability& capability, XnUserID nId, void* pCookie )
    {
        ((SkeletonCallback*)pCookie)->CalibrationStart( capability, nId );
    }

    static void XN_CALLBACK_TYPE CalibrationEnd( xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie )
    {
        ((SkeletonCallback*)pCookie)->CalibrationEnd( capability, nId, bSuccess );
    }

    virtual void CalibrationStart( xn::SkeletonCapability& capability, XnUserID nId )
    {
    }

    virtual void CalibrationEnd( xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess )
    {
    }
};

class Skeleton
{
public:

    Skeleton()
        : skelton_( 0 )
        , calibrationCallbacks_( 0 )
    {
    }

    virtual ~Skeleton()
    {
        if ( skelton_.IsValid() && (calibrationCallbacks_ != 0) ) {
            skelton_.UnregisterCalibrationCallbacks( calibrationCallbacks_ );
        }
    }

    void SetSkeletonCapability( xn::SkeletonCapability skelton ) { skelton_ = skelton; }
    xn::SkeletonCapability& GetSkeletonCapability() { return skelton_; }

    void RegisterCallback( SkeletonCallback* callback )
    {
        assert( skelton_.IsValid() );

        XnStatus rc = skelton_.RegisterCalibrationCallbacks( &SkeletonCallback::CalibrationStart, &SkeletonCallback::CalibrationEnd, callback, calibrationCallbacks_);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }
    }

protected:

    xn::SkeletonCapability skelton_;
    XnCallbackHandle calibrationCallbacks_;
};

