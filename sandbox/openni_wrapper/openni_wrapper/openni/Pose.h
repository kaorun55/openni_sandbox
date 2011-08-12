#pragma once

#include <XnCppWrapper.h>

namespace openni {
    class PoseCallback
    {
        friend class Pose;

    public:

        virtual ~PoseCallback(){}

    protected:

        // ポーズ検出
        static void XN_CALLBACK_TYPE PoseDetected( xn::PoseDetectionCapability& capability,
            const XnChar* strPose, XnUserID nId, void* pCookie )
        {
            ((PoseCallback*)pCookie)->PoseDetected( capability, strPose, nId );
        }

        // ポーズ消失
        static void XN_CALLBACK_TYPE PoseLost( xn::PoseDetectionCapability& capability,
            const XnChar* strPose, XnUserID nId, void* pCookie )
        {
            ((PoseCallback*)pCookie)->PoseLost( capability, strPose, nId );
        }

        // ポーズ検出
        virtual void PoseDetected( xn::PoseDetectionCapability& capability,
            const XnChar* strPose, XnUserID nId )
        {
        }

        // ポーズ消失
        virtual void PoseLost( xn::PoseDetectionCapability& capability,
            const XnChar* strPose, XnUserID nId )
        {
        }
    };


    class Pose
    {
    public:

        Pose()
            : pose_( 0 )
            , poseCallbacks_( 0 )
        {
        }

        ~Pose()
        {
            if ( pose_.IsValid() && (poseCallbacks_ != 0) ) {
                pose_.UnregisterFromPoseCallbacks( poseCallbacks_ );
            }
        }

        void SetPoseDetectionCapability( xn::PoseDetectionCapability pose ) { pose_ = pose; }

        xn::PoseDetectionCapability& GetPoseDetectionCapability() { return pose_; }

        void RegisterCallback( PoseCallback* callback )
        {
            assert( pose_.IsValid() );

            XnStatus rc = pose_.RegisterToPoseCallbacks( &PoseCallback::PoseDetected, &PoseCallback::PoseLost, callback, poseCallbacks_ );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

    protected:

        xn::PoseDetectionCapability pose_;
        XnCallbackHandle poseCallbacks_;

    };
}
