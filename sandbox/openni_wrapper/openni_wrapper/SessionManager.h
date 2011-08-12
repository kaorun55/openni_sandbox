#pragma once

#include <XnCppWrapper.h>
#include <XnVSessionManager.h>

class SessionManagerCallback
{
    friend class SessionManager;

protected:

    static void XN_CALLBACK_TYPE SessionStart( const XnPoint3D& pFocus, void* UserCxt )
    {
        ((SessionManagerCallback*)UserCxt)->SessionStart( pFocus );
    }

    static void XN_CALLBACK_TYPE SessionEnd( void* UserCxt )
    {
        ((SessionManagerCallback*)UserCxt)->SessionEnd();
    }

    virtual void SessionStart( const XnPoint3D& pFocus )
    {
    }

    virtual void SessionEnd()
    {
    }
};

class SessionManager
{
public:

    SessionManager()
        : sessionCallback_( 0 )
    {
    }

    ~SessionManager()
    {
        sessionManager_.UnregisterSession( sessionCallback_ );
    }

    void Initialize( xn::Context& context,
						const std::string& useAsFocus, const std::string& useAsQuickRefocus,
						xn::HandsGenerator* pTracker = 0, xn::GestureGenerator* pFocusGenerator = 0,
						xn::GestureGenerator* pQuickRefocusGenerator = 0 )
    {
        XnStatus rc = sessionManager_.Initialize( &context, useAsFocus.c_str(), useAsQuickRefocus.c_str(),
						                pTracker, pFocusGenerator, pQuickRefocusGenerator );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }
    }

    XnVSessionManager& GetSessionManager() { return sessionManager_; }

    void RegisterCallback( SessionManagerCallback* callback )
    {
        sessionCallback_ = sessionManager_.RegisterSession( callback, &SessionManagerCallback::SessionStart, &SessionManagerCallback::SessionEnd );
    }

protected:

    XnVSessionManager sessionManager_;
    XnVHandle sessionCallback_;
};
