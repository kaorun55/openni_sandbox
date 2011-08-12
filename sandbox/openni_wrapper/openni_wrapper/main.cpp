// Windows の場合はReleaseコンパイルにすると
// 現実的な速度で動作します
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv2/opencv.hpp>

#include <XnCppWrapper.h>
#include "openni/User.h"
#include "openni/Pose.h"
#include "openni/Skeleton.h"

#include "nite/SessionManager.h"
#include "nite/SwipeDetector.h"

#include "SkeltonDrawer.h"

const char* CONFIG_XML_PATH = "SamplesConfig.xml";
const char* RECORDE_PATH = "record.oni";

// ユーザーの色づけ
const XnFloat Colors[][3] =
{
    {1,1,1},    // ユーザーなし
    {0,1,1},  {0,0,1},  {0,1,0},
    {1,1,0},  {1,0,0},  {1,.5,0},
    {.5,1,0}, {0,.5,1}, {.5,0,1},
    {1,1,.5},
};

class App : public openni::UserCallback,
            public openni::PoseCallback,
            public openni::SkeletonCallback,
            public nite::SessionManagerCallback,
            public nite::SwipeDetectorCallback
{
public:

    App()
        : isShowImage( true )
        , isShowUser( true )
        , isShowSkeleton( true )
    {
    }

    App( const std::string& xmlFileName, const std::string& recordFileName )
        : isShowImage( true )
        , isShowUser( true )
        , isShowSkeleton( true )
    {
    }

    App( const std::string& recordFileName )
        : isShowImage( true )
        , isShowUser( true )
        , isShowSkeleton( true )
    {
    }

    void InitFromXml( const std::string& xmlFileName, const std::string& recordFileName )
    {
        // コンテキストの初期化
        XnStatus rc = context.InitFromXmlFile( xmlFileName.c_str() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // レコーダーの作成
        rc = recorder.Create(context);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // 記録設定
        rc = recorder.SetDestination( XN_RECORD_MEDIUM_FILE, recordFileName.c_str() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        InitOpenNI();
        InitOpenCV();
    }

    void InitFromRecord( const std::string& recordFileName )
    {
        XnStatus rc = context.Init();
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // 記録されたファイルを開く
        rc = context.OpenFileRecording( recordFileName.c_str() );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // プレーヤーの作成
        rc = context.FindExistingNode( XN_NODE_TYPE_PLAYER, player );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        InitOpenNI();
        InitOpenCV();
    }

    void Run()
    {
        while ( 1 ) {
            // すべてのノードの更新を待つ
            context.WaitAndUpdateAll();
            sessionManager.Update();

            // 画像データの取得
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // ユーザーデータの取得
            xn::SceneMetaData sceneMD;
            user.GetUserGenerator().GetUserPixels( 0, sceneMD );

            // カメラ画像の表示
            char* dest = camera->imageData;
            const xn::RGB24Map& rgb = imageMD.RGB24Map();
            for (int y = 0; y < imageMD.YRes(); ++y) {
                for (int x = 0; x < imageMD.XRes(); ++x) {
                    // ユーザー表示
                    XnLabel label = sceneMD(x, y);
                    if (!isShowUser) {
                        label = 0;
                    }

                    // カメラ画像の表示
                    XnRGB24Pixel pixel = rgb(x, y);
                    if (!isShowImage) {
                        pixel = xnRGB24Pixel( 255, 255, 255 );
                    }

                    // 出力先に描画
                    dest[0] = pixel.nRed   * Colors[label][0];
                    dest[1] = pixel.nGreen * Colors[label][1];
                    dest[2] = pixel.nBlue  * Colors[label][2];
                    dest += 3;
                }
            }

            // スケルトンの描画
            if ( isShowSkeleton ) {
                XnUserID users[15];
                XnUInt16 userCount = sizeof(users) / sizeof(users[0]);
                user.GetUserGenerator().GetUsers( users, userCount );
                for ( int i = 0; i < userCount; ++i ) {
                    if (skeleton.GetSkeletonCapability().IsTracking( users[i] ) ) {
                        SkeltonDrawer skeltonDrawer( camera, skeleton.GetSkeletonCapability(), depth, users[i] );
                        skeltonDrawer.draw();
                    }
                }
            }

            ::cvCvtColor(camera, camera, CV_BGR2RGB);
            ::cvShowImage("KinectImage", camera);

            // キーイベント
            char key = cvWaitKey(10);
            // 終了する
            if (key == 'q') {
                break;
            }
            // 表示する/しないの切り替え
            else if (key == 'i') {
                isShowImage = !isShowImage;
            }
            else if (key == 'u') {
                isShowUser = !isShowUser;
            }
        }
    }

protected:

    void InitOpenNI()
    {
        // イメージジェネレータの作成
        XnStatus rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // デプスジェネレータの作成
        rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // デプスの座標をイメージに合わせる
        //  ユーザー座標のビューポイントもデプスの座標で合わせる
        depth.GetAlternativeViewPointCap().SetViewPoint( image );

        // ユーザーの作成
        rc = context.FindExistingNode( XN_NODE_TYPE_USER, user.GetUserGenerator() );
        if ( rc != XN_STATUS_OK ) {
            rc = user.GetUserGenerator().Create( context );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

        // ユーザー検出機能をサポートしているか確認
        if ( !user.GetUserGenerator().IsCapabilitySupported( XN_CAPABILITY_SKELETON ) ) {
            throw std::runtime_error( "ユーザー検出をサポートしてません" );
        }


        // キャリブレーションにポーズが必要
        skeleton.SetSkeletonCapability( user.GetUserGenerator().GetSkeletonCap() );
        if ( skeleton.GetSkeletonCapability().NeedPoseForCalibration() ) {
            // ポーズ検出のサポートチェック
            if ( !user.GetUserGenerator().IsCapabilitySupported( XN_CAPABILITY_POSE_DETECTION ) ) {
                throw std::runtime_error( "ポーズ検出をサポートしてません" );
            }

            // キャリブレーションポーズの取得
            XnChar p[20] = "";
            skeleton.GetSkeletonCapability().GetCalibrationPose( p );
            poseName = p;

            // ポーズ検出のコールバックを登録
            pose.SetPoseDetectionCapability( user.GetUserGenerator().GetPoseDetectionCap() );
            pose.RegisterCallback( this );
        }

        // ユーザー認識のコールバックを登録
        user.RegisterCallback( this );

        // キャリブレーションのコールバックを登録
        skeleton.RegisterCallback( this );

        // ユーザートラッキングで、すべてをトラッキングする
        skeleton.GetSkeletonCapability().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

        // 記録する場合は、ジェネレータの設定
        if ( recorder.IsValid() ) {
            // イメージを記録対象に追加
            rc = recorder.AddNodeToRecording( image, XN_CODEC_JPEG );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        
            // デプスを記録対象に追加
            rc = recorder.AddNodeToRecording( depth, XN_CODEC_UNCOMPRESSED );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        
            // 記録開始(WaitOneUpdateAllのタイミングで記録される)
            rc = recorder.Record();
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }
        }

        sessionManager.Initialize( context, "Wave,Click,RaiseHand", "RaiseHand" );
        sessionManager.RegisterCallback( this );

        swipeDetector.RegisterCallback( this );
        sessionManager.GetSessionManager().AddListener( &swipeDetector.GetSwipeDetector() );

        // ジェスチャー検出の開始
        context.StartGeneratingAll();
    }

    void InitOpenCV()
    {
        // カメラサイズのイメージを作成(8bitのRGB)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode( outputMode );
        camera = ::cvCreateImage( cvSize( outputMode.nXRes, outputMode.nYRes ), IPL_DEPTH_8U, 3 );
        if ( !camera ) {
            throw std::runtime_error( "error : cvCreateImage" );
        }
    }

protected:

    virtual void UserDetected( xn::UserGenerator& generator, XnUserID nId )
    {
        std::cout << "ユーザー検出:" << nId << " " << generator.GetNumberOfUsers() << "人目" << std::endl;
    
        if ( !poseName.empty() ) {
            generator.GetPoseDetectionCap().StartPoseDetection( poseName.c_str(), nId );
        }
        else {
            generator.GetSkeletonCap().RequestCalibration( nId, TRUE );
        }
    }

    virtual void UserLost( xn::UserGenerator& generator, XnUserID nId )
    {
        std::cout << "ユーザー消失:" << nId << std::endl;
    }

    // ポーズ検出
    void PoseDetected( xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId )
    {
      std::cout << "ポーズ検出:" << strPose << " ユーザー:" << nId << std::endl;

      user.GetUserGenerator().GetPoseDetectionCap().StopPoseDetection( nId );
      user.GetUserGenerator().GetSkeletonCap().RequestCalibration( nId, TRUE );
    }

    // ポーズ消失
    void PoseLost( xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId)
    {
      std::cout << "ポーズ消失:" << strPose << " ユーザー:" << nId << std::endl;
    }

    // キャリブレーションの開始
    void CalibrationStart( xn::SkeletonCapability& capability, XnUserID nId)
    {
      std::cout << "キャリブレーション開始。ユーザー:" << nId << std::endl;
    }

    // キャリブレーションの終了
    void CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess )
    {
      // キャリブレーション成功
      if (bSuccess) {
        std::cout << "キャリブレーション成功。ユーザー:" << nId << std::endl;
        user.GetUserGenerator().GetSkeletonCap().StartTracking( nId );
      }
      // キャリブレーション失敗
      else {
        std::cout << "キャリブレーション失敗。ユーザー:" << nId << std::endl;
      }
    }

    virtual void SessionStart( const XnPoint3D& pFocus )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SessionEnd()
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeUp( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeDown( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeRight( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void SwipeLeft( XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    virtual void Swipe( XnVDirection eDir, XnFloat fVelocity, XnFloat fAngle )
    {
        std::cout << __FUNCTION__ << std::endl;
    }

private:
    
    // RGBピクセルの初期化
    inline XnRGB24Pixel xnRGB24Pixel( int r, int g, int b )
    {
        XnRGB24Pixel pixel = { r, g, b };
        return pixel;
    }

private:

    xn::Context context;
    xn::Recorder recorder;
    xn::Player player;
    xn::ImageGenerator image;
    xn::DepthGenerator depth;

    openni::User user;
    openni::Pose pose;
    openni::Skeleton skeleton;

    nite::SessionManager sessionManager;
    nite::SwipeDetector swipeDetector;

    cv::Ptr< IplImage >camera;

    std::string poseName;

    // 表示状態
    bool isShowImage;
    bool isShowUser;
    bool isShowSkeleton;
};

int main (int argc, char * argv[])
{
    try {
        App app;
        if ( argc == 1 ) {
            app.InitFromXml( CONFIG_XML_PATH, RECORDE_PATH );
        }
        else {
            app.InitFromRecord( argv[1] );
        }

        app.Run();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
