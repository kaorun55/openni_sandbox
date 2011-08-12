#pragma once

#include <XnCppWrapper.h>

namespace openni {
    class UserCallback
    {
        friend class User;

    public:

        virtual ~UserCallback(){}

    protected:

        static void XN_CALLBACK_TYPE UserDetected( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
        {
            ((UserCallback*)pCookie)->UserDetected( generator, nId );
        }

        static void XN_CALLBACK_TYPE UserLost( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
        {
            ((UserCallback*)pCookie)->UserLost( generator, nId );
        }

        virtual void UserDetected( xn::UserGenerator& generator, XnUserID nId )
        {
        }

        virtual void UserLost( xn::UserGenerator& generator, XnUserID nId )
        {
        }
    };

    class User
    {
    public:

        User()
            : userCallbacks_( 0 )
        {
        }

        virtual ~User()
        {
            if ( user_.IsValid() && (userCallbacks_ != 0) ) {
                user_.UnregisterUserCallbacks( userCallbacks_ );
            }
        }

        xn::UserGenerator& GetUserGenerator() { return user_; }

        void RegisterCallback( UserCallback* callback )
        {
            XnStatus rc = user_.RegisterUserCallbacks( &UserCallback::UserDetected, &UserCallback::UserLost, callback, userCallbacks_);
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

    protected:

        xn::UserGenerator user_;
        XnCallbackHandle userCallbacks_;
    };
}
