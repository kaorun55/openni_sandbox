#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <XnCppWrapper.h>

const XnMapOutputMode OUTPUT_MODE = { 640, 480, 30 };

// Kinectごとの表示情報
struct Kinect
{
	xn::ImageGenerator  image;
	xn::DepthGenerator  depth;
	IplImage*           camera;
	xn::MapGenerator*   generator;
};

// デプスのヒストグラムを作成
typedef std::vector<float> depth_hist;
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

// 検出されたデバイスを列挙する
void EnumerateProductionTrees(xn::Context& context, XnProductionNodeType type)
{
	xn::NodeInfoList nodes;
	XnStatus rc = context.EnumerateProductionTrees(type, NULL, nodes);
	if (rc != XN_STATUS_OK) {
		std::cout << ::xnGetStatusString(rc) << std::endl;
		return;
	}
	else if (nodes.Begin () == nodes.End ()) {
		std::cout << "No devices found." << std::endl;
		return;
	}

	for (xn::NodeInfoList::Iterator it = nodes.Begin (); it != nodes.End (); ++it) {
		std::cout <<
			::xnProductionNodeTypeToString( (*it).GetDescription().Type ) <<
			", " <<
			(*it).GetCreationInfo() << ", " <<
			(*it).GetInstanceName() << ", " <<
			(*it).GetDescription().strName << ", " <<
			(*it).GetDescription().strVendor << ", " <<
			std::endl;

		xn::NodeInfo info = *it;
		context.CreateProductionTree(info);
	}
}

// ジェネレータを作成する
template<typename T>
T CreateGenerator(const xn::NodeInfo& node)
{
	T g;
	XnStatus rc = node.GetInstance(g);
	if (rc != XN_STATUS_OK) {
		throw std::runtime_error(xnGetStatusString(rc));
	}

	g.SetMapOutputMode(OUTPUT_MODE);

	return g;
}

int main (int argc, char * argv[])
{
	try {
		XnStatus rc;

		xn::Context context;
		rc = context.Init();
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}

		// 検出されたデバイスを利用可能として登録する
		EnumerateProductionTrees(context, XN_NODE_TYPE_DEVICE);
		EnumerateProductionTrees(context, XN_NODE_TYPE_IMAGE);
		EnumerateProductionTrees(context, XN_NODE_TYPE_DEPTH);

		// 登録されたデバイスを取得する
		std::cout << "xn::Context::EnumerateExistingNodes ... ";
		xn::NodeInfoList nodeList;
		rc = context.EnumerateExistingNodes( nodeList );
		if ( rc != XN_STATUS_OK ) {
			throw std::runtime_error( ::xnGetStatusString( rc ) );
		}
		std::cout << "Success" << std::endl;

		if ( nodeList.IsEmpty() ) {
			throw std::runtime_error( "No devices found." );
		}

		// 登録されたデバイスからジェネレータを生成する
		std::map<int, Kinect> kinect;
		for ( xn::NodeInfoList::Iterator it = nodeList.Begin();
			it != nodeList.End(); ++it ) {

				// インスタンス名の最後が番号になっている
				std::string name = (*it).GetInstanceName();
				int no = *name.rbegin() - '1';

				std::cout <<
					::xnProductionNodeTypeToString( (*it).GetDescription().Type ) <<
					", " <<
					(*it).GetCreationInfo() << ", " <<
					(*it).GetInstanceName() << ", " <<
					(*it).GetDescription().strName << ", " <<
					(*it).GetDescription().strVendor << ", " <<
					no << ", " << 
					std::endl;

				if ((*it).GetDescription().Type == XN_NODE_TYPE_IMAGE) {
					kinect[no].image = CreateGenerator<xn::ImageGenerator>(*it);
					kinect[no].generator = &kinect[no].image;
				}
				else if ((*it).GetDescription().Type == XN_NODE_TYPE_DEPTH) {
					kinect[no].depth = CreateGenerator<xn::DepthGenerator>(*it);
					kinect[no].generator = &kinect[no].depth;
				}
		}

		// ジェネレートを開始する
		context.StartGeneratingAll();

		// ビューポイントの設定や、カメラ領域を作成する
		for (std::map<int, Kinect>::iterator it = kinect.begin(); it != kinect.end(); ++it) {
			int no = it->first;
			Kinect& k = it->second;
			k.depth.GetAlternativeViewPointCap().SetViewPoint(*k.generator);


			::cvNamedWindow( k.generator->GetName() );
			::cvResizeWindow( k.generator->GetName(), OUTPUT_MODE.nXRes, OUTPUT_MODE.nYRes );
			::cvMoveWindow( k.generator->GetName(), (no % 2) * OUTPUT_MODE.nXRes, (no / 2) * OUTPUT_MODE.nYRes );

			kinect[no].camera = ::cvCreateImage(cvSize(OUTPUT_MODE.nXRes, OUTPUT_MODE.nYRes),
				IPL_DEPTH_8U, 3);
			if (!kinect[no].camera) {
				throw std::runtime_error("error : cvCreateImage");
			}
		}

		// メインループ
		bool shouldRun = true;
		while ( shouldRun ) {
			context.WaitAndUpdateAll();

			// 検出したすべてのKinectの画像を表示する
			for (std::map<int, Kinect>::iterator it = kinect.begin(); it != kinect.end(); ++it) {
				Kinect& k = it->second;

				xn::ImageMetaData imageMD;
				if ( k.image.IsValid() ) {
					k.image.GetMetaData(imageMD);
				}

				xn::DepthMetaData depthMD;
				if ( k.depth.IsValid() ) {
					k.depth.GetMetaData(depthMD);
				}

				// デプスマップの作成
				depth_hist depthHist = getDepthHistgram(k.depth, depthMD);

				// イメージをデプスマップで上書きする
				XnRGB24Pixel* rgb = (XnRGB24Pixel*)k.camera->imageData;
				for (XnUInt y = 0; y < k.camera->height; ++y) {
					for (XnUInt x = 0; x < k.camera->width; ++x, ++rgb) {
						const XnDepthPixel& depth = depthMD(x, y);
						XnRGB24Pixel& pixel = *rgb;
						if (depth != 0) {
							pixel.nRed   = depthHist[depthMD(x, y)];
							pixel.nGreen = depthHist[depthMD(x, y)];
							pixel.nBlue  = 0;
						}
						else if ( k.image.IsValid() ) {
							pixel = imageMD.RGB24Map()( x, y );
						}
					}
				}

				// カメラ画像の表示
				::cvCvtColor(k.camera, k.camera, CV_BGR2RGB);
				::cvShowImage(k.generator->GetName(), k.camera);

				// 連続してcvShowImageを呼び出すとすべてのウィンドウで最後のデータが表示されてしまうので、
				// OpenCVのWait機能を利用して少し待つ
				// キーの取得
				char key = ::cvWaitKey(10);
				// 終了する
				if (key == 'q') {
					shouldRun = false;
				}
			}
		}
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}

	return 0;
}
