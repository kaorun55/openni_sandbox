#pragma once

#include <XnCppWrapper.h>

namespace openni {
    class PlayerCallback
    {
        friend class Player;

    public:

        virtual ~PlayerCallback(){}

    protected:

        // ファイルの終端
        static void XN_CALLBACK_TYPE EndOfFileReached( xn::ProductionNode& node, void* pCookie)
        {
            ((PlayerCallback*)pCookie)->EndOfFileReached( node );
        }

        // ファイルの終端
        virtual void EndOfFileReached( xn::ProductionNode& node )
        {
        }
    };


    class Player
    {
    public:

        Player()
            : callback_( 0 )
        {
        }

        ~Player()
        {
            if ( player_.IsValid() && (callback_ != 0) ) {
                player_.UnregisterFromEndOfFileReached( callback_ );
            }
        }

        xn::Player& GetPlayer() { return player_; }

        void RegisterCallback( PlayerCallback* callback )
        {
            assert( player_.IsValid() );

            XnStatus rc = player_.RegisterToEndOfFileReached( &PlayerCallback::EndOfFileReached, callback, callback_ );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

    protected:

        xn::Player player_;
        XnCallbackHandle callback_;

    };
}
