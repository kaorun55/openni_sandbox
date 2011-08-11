#include <iostream>
#include <stdexcept>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <XnCppWrapper.h>

// 設定ファイルのパス(環境に合わせて変更してください)
const char* CONFIG_XML_PATH = "SamplesConfig.xml";

int main (int argc, char * argv[])
{
    IplImage* camera = 0;

    try {
        // コンテキストの初期化 ... (1)
        xn::Context context;
        XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // イメージジェネレータの作成 ... (2)
        xn::ImageGenerator image;
        rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE, image);
        if (rc != XN_STATUS_OK) {
            throw std::runtime_error(xnGetStatusString(rc));
        }

        // カメラサイズのイメージを作成(8bitのRGB) ... (3)
        XnMapOutputMode outputMode;
        image.GetMapOutputMode(outputMode);
        camera = ::cvCreateImage(cvSize(outputMode.nXRes, outputMode.nYRes),
            IPL_DEPTH_8U, 3);
        if (!camera) {
            throw std::runtime_error("error : cvCreateImage");
        }

        // メインループ
        while (1) {
            // カメライメージの更新を待ち、画像データを取得する ... (4)
            context.WaitOneUpdateAll(image);
            xn::ImageMetaData imageMD;
            image.GetMetaData(imageMD);

            // カメラ画像の表示 ... (5)
            //  Kinectからの入力がRGBであるため、BGRに変換して表示する
            memcpy(camera->imageData, imageMD.RGB24Data(), camera->imageSize);
            ::cvCvtColor(camera, camera, CV_RGB2BGR);
            ::cvShowImage("openni_opencv21", camera);

            // キーの取得
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

    ::cvReleaseImage(&camera);

    return 0;
}
