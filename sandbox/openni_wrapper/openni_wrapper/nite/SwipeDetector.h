#pragma once

#include <XnCppWrapper.h>
#include <XnVSwipeDetector.h>

namespace nite {
    class SwipeDetectorCallback
    {
        friend class SwipeDetector;

    public:

        virtual ~SwipeDetectorCallback()
        {
        }

    protected:

        static void XN_CALLBACK_TYPE SwipeUp( XnFloat fVelocity, XnFloat fAngle, void *pUserCxt )
        {
            ((SwipeDetectorCallback*)pUserCxt)->SwipeUp( fVelocity, fAngle );
        }

        static void XN_CALLBACK_TYPE SwipeDown( XnFloat fVelocity, XnFloat fAngle, void *pUserCxt )
        {
            ((SwipeDetectorCallback*)pUserCxt)->SwipeDown( fVelocity, fAngle );
        }

        static void XN_CALLBACK_TYPE SwipeRight( XnFloat fVelocity, XnFloat fAngle, void *pUserCxt )
        {
            ((SwipeDetectorCallback*)pUserCxt)->SwipeRight( fVelocity, fAngle );
        }

        static void XN_CALLBACK_TYPE SwipeLeft( XnFloat fVelocity, XnFloat fAngle, void *pUserCxt )
        {
            ((SwipeDetectorCallback*)pUserCxt)->SwipeLeft( fVelocity, fAngle );
        }

        static void XN_CALLBACK_TYPE Swipe( XnVDirection eDir, XnFloat fVelocity, XnFloat fAngle, void *pUserCxt )
        {
            ((SwipeDetectorCallback*)pUserCxt)->Swipe( eDir, fVelocity, fAngle );
        }

        virtual void SwipeUp( XnFloat fVelocity, XnFloat fAngle )
        {
        }

        virtual void SwipeDown( XnFloat fVelocity, XnFloat fAngle )
        {
        }

        virtual void SwipeRight( XnFloat fVelocity, XnFloat fAngle )
        {
        }

        virtual void SwipeLeft( XnFloat fVelocity, XnFloat fAngle )
        {
        }

        virtual void Swipe( XnVDirection eDir, XnFloat fVelocity, XnFloat fAngle )
        {
        }
    };

    class SwipeDetector
    {
    public:

        SwipeDetector()
            : swipeUpCallback_( 0 )
            , swipeDownCallback_( 0 )
            , swipeRightCallback_( 0 )
            , swipeLeftCallback_( 0 )
            , swipeCallback_( 0 )
        {
        }

        ~SwipeDetector()
        {
            swipeDetector_.UnregisterSwipeUp( swipeUpCallback_ );
            swipeDetector_.UnregisterSwipeDown( swipeDownCallback_ );
            swipeDetector_.UnregisterSwipeRight( swipeRightCallback_ );
            swipeDetector_.UnregisterSwipeLeft( swipeLeftCallback_ );
            swipeDetector_.UnregisterSwipe( swipeCallback_ );
        }

        XnVSwipeDetector& GetSwipeDetector() { return swipeDetector_; }

        void RegisterCallback( SwipeDetectorCallback* callback )
        {
            swipeUpCallback_ = swipeDetector_.RegisterSwipeUp( callback, &SwipeDetectorCallback::SwipeUp );
            swipeDownCallback_ = swipeDetector_.RegisterSwipeDown( callback, &SwipeDetectorCallback::SwipeDown );
            swipeRightCallback_ = swipeDetector_.RegisterSwipeRight( callback, &SwipeDetectorCallback::SwipeRight );
            swipeLeftCallback_ = swipeDetector_.RegisterSwipeLeft( callback, &SwipeDetectorCallback::SwipeLeft );
            swipeCallback_ = swipeDetector_.RegisterSwipe( callback, &SwipeDetectorCallback::Swipe );
        }

    protected:

        XnVSwipeDetector swipeDetector_;
        XnCallbackHandle swipeUpCallback_;
        XnCallbackHandle swipeDownCallback_;
        XnCallbackHandle swipeRightCallback_;
        XnCallbackHandle swipeLeftCallback_;
        XnCallbackHandle swipeCallback_;
    };
}
