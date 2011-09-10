#include <gtest\gtest.h>
#include <gmock\gmock.h>

#include "openni\Player.h"

namespace {
    const std::string RECORD_FILE_NAME = "EndOfFileReached.oni";

    // アプリケーションクラス
    class App : public ::openni::PlayerCallback
    {
    public:

        App()
        {
            // プレーヤーの作成と設定
            context.Init();
            context.OpenFileRecording( RECORD_FILE_NAME.c_str() );
            context.FindExistingNode( XN_NODE_TYPE_PLAYER, player.GetPlayer() );
            player.RegisterCallback( this );
            player.GetPlayer().SetRepeat( false );
        }

        void Run()
        {
            // EOFまで回す
            while ( !player.GetPlayer().IsEOF() ) {
                context.WaitAndUpdateAll();
            }
        }

        // ファイルの終端
        virtual void EndOfFileReached( xn::ProductionNode& node )
        {
            std::cout << "ファイルの終端" << std::endl;
        }

    private:

        xn::Context context;
        openni::Player player;
    };

    // アプリケーションクラスのモック
    class MockApp : public App
    {
    public:

        MOCK_METHOD1( EndOfFileReached, void( xn::ProductionNode& node ) );
    };
}

TEST( PlayerTest, EndOfFileReached )
{
    using ::testing::_;

    MockApp app;

    // モックの設定
    EXPECT_CALL( app, EndOfFileReached(_) )
        .Times( 1 );        // ファイルの終端検出は1回呼び出される
//        .Times( 2 );      // 2回はNG

    app.Run();
}
