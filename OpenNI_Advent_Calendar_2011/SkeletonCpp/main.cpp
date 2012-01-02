// Windows の場合はReleaseコンパイルにすると
// 現実的な速度で動作します
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv2/opencv.hpp>

#include <XnCppWrapper.h>

// ユーザー検出
void XN_CALLBACK_TYPE UserDetected( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
{
    std::cout << "ユーザー検出:" << nId << " " << generator.GetNumberOfUsers() << "人目" << std::endl;

    generator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// キャリブレーションの終了
void XN_CALLBACK_TYPE CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
    // キャリブレーション成功
    if ( bSuccess ) {
        std::cout << "キャリブレーション成功。ユーザー:" << nId << std::endl;
        capability.StartTracking(nId);
    }
    // キャリブレーション失敗
    else {
        std::cout << "キャリブレーション失敗。ユーザー:" << nId << std::endl;
    }
}

int main (int argc, char * argv[])
{
    try {
        cv::Ptr< IplImage > camera = 0;

        // コンテキストの初期化
        xn::Context context;
        XnStatus rc = context.InitFromXmlFile("SamplesConfig.xml");
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // イメージジェネレータの作成
        xn::ImageGenerator image;
        rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // デプスジェネレータの作成
        xn::DepthGenerator depth;
        rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // デプスの座標をイメージに合わせる
        depth.GetAlternativeViewPointCap().SetViewPoint(image);

        // ユーザーの作成
        xn::UserGenerator user;
        rc = context.FindExistingNode( XN_NODE_TYPE_USER, user );
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        // スケルトン・トラッキングをサポートしているか確認
        if (!user.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
            throw std::runtime_error("ユーザー検出をサポートしてません");
        }

        // キャリブレーションにポーズが必要
        xn::SkeletonCapability skeleton = user.GetSkeletonCap();
        if ( skeleton.NeedPoseForCalibration() ) {
            throw std::runtime_error("最新のOpenNIをインストールしてください");
        }

        // ユーザー認識のコールバックを登録
        // キャリブレーションのコールバックを登録
        XnCallbackHandle userCallbacks, calibrationCallbacks;
        user.RegisterUserCallbacks(&::UserDetected, 0, 0, userCallbacks);
        skeleton.RegisterCalibrationCallbacks( 0, &::CalibrationEnd, 0, calibrationCallbacks );

        // ユーザートラッキングで、すべてをトラッキングする
        //XN_SKEL_PROFILE_ALL           すべてをトラッキングする
        //XN_SKEL_PROFILE_UPPER         上半身をトラッキングする
        //XN_SKEL_PROFILE_LOWER         下半身をトラッキングする
        //XN_SKEL_PROFILE_HEAD_HANDS    頭と手をトラッキングする
        skeleton.SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

        // カメラサイズのイメージを作成(8bitのRGB)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode(outputMode);
        camera = ::cvCreateImage(cvSize(outputMode.nXRes, outputMode.nYRes), IPL_DEPTH_8U, 3);
        if (!camera) {
            throw std::runtime_error("error : cvCreateImage");
        }

        // メインループ
        while (1) {
            // すべてのノードの更新を待つ
            context.WaitAndUpdateAll();

            // 画像データの取得
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // ユーザーデータの取得
            xn::SceneMetaData sceneMD;
            user.GetUserPixels(0, sceneMD);

            // カメラ画像の表示
            memcpy( camera->imageData, imageMD.Data(), camera->imageSize );

            // スケルトンの描画
            XnUserID users[15];
            XnUInt16 userCount = 15;
            user.GetUsers(users, userCount);
            for (int i = 0; i < userCount; ++i) {
                if ( !skeleton.IsTracking( users[i] ) ) {
                    continue;
                }

                for ( int j = (int)XN_SKEL_HEAD; j <= (int)XN_SKEL_RIGHT_FOOT; ++j ) {
                    if ( !skeleton.IsJointAvailable( (XnSkeletonJoint)j ) ) {
                        continue;
                    }

                    // 各箇所の座標を取得する
                    XnSkeletonJointPosition joint;
                    skeleton.GetSkeletonJointPosition(users[i], (XnSkeletonJoint)j, joint);
                    if ( joint.fConfidence < 0.5 ) {
                        continue;
                    }

                    // 座標を変換する
                    XnPoint3D pt = joint.position;
                    depth.ConvertRealWorldToProjective( 1, &pt, &pt );
                    cvCircle( camera, cvPoint(pt.X, pt.Y), 10, cvScalar( 255, 0, 0 ), -1 );
                }
            }

            ::cvCvtColor(camera, camera, CV_RGB2BGR);
            ::cvShowImage("KinectImage", camera);

            // キーイベント
            char key = cvWaitKey(10);
            // 終了する
            if (key == 'q') {
                break;
            }
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
