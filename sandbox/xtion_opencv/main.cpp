#include <XnCppWrapper.h>

#include <iostream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <ctype.h>

// デプスのヒストグラムを作成
typedef std::vector<float> depth_hist;
depth_hist getDepthHistgram(const xn::DepthGenerator& depth,
	const xn::DepthMetaData& depthMD)
{
	if ( !depth.IsValid() ) {
		return depth_hist();
	}

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

int	main (int argc, char **argv)
{
	CvCapture *capture = 0;
	IplImage *frame = 0;
	double w = 320, h = 240;
	int c;

	try {
		XnStatus rc;

		xn::Context context;
		rc = context.InitFromXmlFile("SamplesConfig.xml");
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		xn::DepthGenerator  depthGenerator;
		rc = context.FindExistingNode( XN_NODE_TYPE_DEPTH, depthGenerator );
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		// ジェネレートを開始する
		context.StartGeneratingAll();

		// カメラの画像を取得する
		capture = cvCreateCameraCapture( 0 );
		if ( capture == 0 ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}

		cvNamedWindow ("Capture", CV_WINDOW_AUTOSIZE);

		bool isDepth = true;

		// (3)カメラから画像をキャプチャする
		while (1) {
			context.WaitAndUpdateAll();

			frame = cvQueryFrame (capture);

			// デプスマップの作成
			xn::DepthMetaData depthMD;
			depthGenerator.GetMetaData(depthMD);

			if ( isDepth ) {
				// デプスマップを描画する
				depth_hist depthHist = getDepthHistgram( depthGenerator, depthMD );
				XnRGB24Pixel* rgb = (XnRGB24Pixel*)frame->imageData;
				for (XnUInt y = 0; y < frame->height; ++y) {
					for (XnUInt x = 0; x < frame->width; ++x, ++rgb) {
						// デプスマップまたはイメージを描画する
						XnRGB24Pixel& pixel = *rgb;
						if ( depthMD( x, y ) != 0 ) {
							pixel.nRed   = depthHist[depthMD( x, y )];
							pixel.nGreen = depthHist[depthMD( x, y )];
							pixel.nBlue  = 0;
						}
					}
				}
			}

			cvShowImage ("Capture", frame);
			c = cvWaitKey (2);
			if (c == '\x1b') {
				break;
			}
			else if ( c == 'd' ) {
				isDepth = !isDepth;
			}
		}
	}
	catch ( std::exception& ex ) {
		std::cout << ex.what() << std::endl;
	}

	cvReleaseCapture (&capture);
	cvDestroyWindow ("Capture");

	return 0;
}
