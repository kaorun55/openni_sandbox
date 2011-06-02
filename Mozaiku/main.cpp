#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <XnCppWrapper.h>

// 設定ファイルのパス(環境に合わせて変更してください)
#if (XN_PLATFORM == XN_PLATFORM_WIN32)
const char* CONFIG_XML_PATH = "../../../Data/SamplesConfig.xml";
#elif (XN_PLATFORM == XN_PLATFORM_MACOSX)
const char* CONFIG_XML_PATH = "../../../Data/SamplesConfig.xml";
#elif (XN_PLATFORM == XN_PLATFORM_LINUX_X86)
const char* CONFIG_XML_PATH = "../../../Data/SamplesConfig.xml";
#else
const char* CONFIG_XML_PATH = "Data/SamplesConfig.xml";
#endif

typedef std::vector<float> depth_hist;

// デプスのヒストグラムを作成
depth_hist getDepthHistgram(const xn::DepthGenerator& depth,
                            const xn::DepthMetaData& depthMD)
{
  // デプスの傾向を計算する(アルゴリズムはNiSimpleViewer.cppを利用)
  const int MAX_DEPTH = depth.GetDeviceMaxDepth();
  depth_hist depthHist(MAX_DEPTH);

  unsigned int points = 0;
  const XnDepthPixel* pDepth = depthMD.Data();
  for (XnUInt y = 0; y < depthMD.YRes(); ++y) {
    for (XnUInt x = 0; x < depthMD.XRes(); ++x, ++pDepth) {
      if (*pDepth != 0) {
        depthHist[*pDepth]++;
        points++;
      }
    }
  }

  for (int i = 1; i < MAX_DEPTH; ++i) {
    depthHist[i] += depthHist[i-1];
  }

  if ( points != 0) {
    for (int i = 1; i < MAX_DEPTH; ++i) {
      depthHist[i] =
        (unsigned int)(256 * (1.0f - (depthHist[i] / points)));
    }
  }

  return depthHist;
}

int main (int argc, char * argv[])
{
  IplImage* camera = 0;

  try {
    // コンテキストの作成
    xn::Context context;
    XnStatus rc = context.InitFromXmlFile(CONFIG_XML_PATH);
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
    xn::AlternativeViewPointCapability viewPoint =
                                depth.GetAlternativeViewPointCap();
    viewPoint.SetViewPoint(image);

    // カメラサイズのイメージを作成(8bitのRGB)
    xn::ImageMetaData imageMD;
    image.GetMetaData(imageMD);
    camera = ::cvCreateImage(cvSize(imageMD.XRes(), imageMD.YRes()),
      IPL_DEPTH_8U, 3);
    if (!camera) {
      throw std::runtime_error("error : cvCreateImage");
    }

    // メインループ
    while (1) {
      // すべての更新を待ち、画像およびデプスデータを取得する
      context.WaitAndUpdateAll();

      xn::ImageMetaData imageMD;
      image.GetMetaData(imageMD);

      xn::DepthMetaData depthMD;
      depth.GetMetaData(depthMD);

      // デプスマップの作成
      depth_hist depthHist = getDepthHistgram(depth, depthMD);

      // 一定以上の距離は描画しない
      xn::RGB24Map& rgb = imageMD.WritableRGB24Map();
      for (XnUInt y = 0; y < imageMD.YRes(); ++y) {
        for (XnUInt x = 0; x < imageMD.XRes(); ++x) {
          const XnDepthPixel& depth = depthMD(x, y);
          if (depth > 1000) {
            XnRGB24Pixel& pixel = rgb(x, y);
            pixel.nRed   = 255;
            pixel.nGreen = 255;
            pixel.nBlue  = 255;
          }
        }
      }

      // 画像の表示
      memcpy(camera->imageData, imageMD.RGB24Data(), camera->imageSize);
      ::cvCvtColor(camera, camera, CV_RGB2BGR);
      ::cvShowImage("KinectImage", camera);

      // 終了する
      char key = cvWaitKey(10);
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
