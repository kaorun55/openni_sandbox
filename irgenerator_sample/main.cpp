// note : http://nma.web.nitech.ac.jp/fukushima/openni/NICVSampleIR.cpp
#include <opencv2/opencv.hpp>
#include <XnCppWrapper.h>

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
	    printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
	    return rc;													\
	}

int main()
{
	XnStatus nRetVal = XN_STATUS_OK; 

    xn::Context context; 
	nRetVal = context.Init(); 
	CHECK_RC(nRetVal, "Initialize context"); 

    xn::IRGenerator ir; 
	nRetVal = ir.Create(context); 

	XnMapOutputMode mapMode; 
	mapMode.nXRes = 640; 
	mapMode.nYRes = 480; 
	mapMode.nFPS = 30; 
	nRetVal = ir.SetMapOutputMode(mapMode); 

	nRetVal = context.StartGeneratingAll(); 

    xn::IRMetaData irMD; 
    int key = 0;
	while ( key != 'q' ) {
		context.WaitAnyUpdateAll();

		ir.GetMetaData(irMD); 
		const XnIRPixel* pIrRow = irMD.Data(); 

		//for opencv Mat
		cv::Mat ir16(480,640,CV_16SC1,(unsigned short*)irMD.WritableData());
		cv::Mat irMat;
		ir16.convertTo(irMat,CV_8U);

		cv::imshow("ShowImage", irMat); 

		key = cv::waitKey(1);
	}
}
