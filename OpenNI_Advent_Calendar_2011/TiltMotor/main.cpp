// thanks : http://akatukisiden.wordpress.com/2011/12/10/openni%E3%81%A7kinect%E3%81%AE%E3%83%81%E3%83%AB%E3%83%88%E3%83%A2%E3%83%BC%E3%82%BF%E3%83%BC%E9%A6%96%E3%82%92%E5%8B%95%E3%81%8B%E3%81%99/
#include <iostream>
#include <XnUSB.h>
#include <XnOpenNI.h>
#include <XnCppWrapper.h>

enum
{
    VID_MICROSOFT = 0x45e,
    PID_NUI_MOTOR = 0x02b0
};

XN_USB_DEV_HANDLE dev;

int main() {

    xn::Context context;
    int ret = context.InitFromXmlFile("SamplesConfig.xml");
    if ( ret != XN_STATUS_OK ) {
        std::cout << xnGetStatusString( ret ) <<std::endl;
    }

    XnUInt16 angle = 20;
    const XnUSBConnectionString* pastrDevicePaths = NULL;
    XnUInt32 nCount = 0;
    XnUChar empty[1];

    xnUSBInit();
    xnUSBEnumerateDevices(VID_MICROSOFT, PID_NUI_MOTOR,&pastrDevicePaths,&nCount);
    xnUSBOpenDeviceByPath(*pastrDevicePaths, &dev);

    xnUSBSendControl( dev, XN_USB_CONTROL_TYPE_VENDOR, 0x31,angle * 2, 0, empty, 0, 0 );

    xnUSBCloseDevice(dev);
    xnUSBFreeDevicesList(pastrDevicePaths);
    xnUSBShutdown();

    return 0;
}
//--------------------------------------