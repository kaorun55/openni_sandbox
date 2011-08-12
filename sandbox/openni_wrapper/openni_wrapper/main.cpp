// Windows の場合はReleaseコンパイルにすると
// 現実的な速度で動作します
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv2/opencv.hpp>

#include <XnCppWrapper.h>
#include "User.h"

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

class App : public UserCallback,
            public PoseCallback
{
public:

    App()
        : isShowImage( true )
        , isShowUser( true )
    {
    }

    App( const std::string& xmlFileName, const std::string& recordFileName )
        : isShowImage( true )
        , isShowUser( true )
    {
    }

    App( const std::string& recordFileName )
        : isShowImage( true )
        , isShowUser( true )
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

            // 画像データの取得
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // ユーザーデータの取得
            xn::SceneMetaData sceneMD;
            user.GetUserGenerator().GetUserPixels(0, sceneMD);

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
        xn::SkeletonCapability skelton = user.GetUserGenerator().GetSkeletonCap();
        if ( skelton.NeedPoseForCalibration() ) {
            // ポーズ検出のサポートチェック
            if ( !user.GetUserGenerator().IsCapabilitySupported( XN_CAPABILITY_POSE_DETECTION ) ) {
                throw std::runtime_error( "ポーズ検出をサポートしてません" );
            }

            // キャリブレーションポーズの取得
            XnChar p[20] = "";
            skelton.GetCalibrationPose( p );
            poseName = p;

            // ポーズ検出のコールバックを登録
            pose.SetPoseDetectionCapability( user.GetUserGenerator().GetPoseDetectionCap() );
            pose.RegisterCallback( this );
        }

        // ユーザー認識のコールバックを登録
        user.RegisterCallback( this );


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

    User user;
    Pose pose;

    cv::Ptr< IplImage >camera;

    std::string poseName;

    // 表示状態
    bool isShowImage;
    bool isShowUser;
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
