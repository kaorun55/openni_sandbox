// TiltMotorCLI.h

#pragma once

#include <XnUSB.h>

using namespace System;

namespace OpenNI {

    class NTiltMotor
    {
    public: 

        NTiltMotor()
        {
            XnUInt32 nCount = 0;

            xnUSBInit();
            xnUSBEnumerateDevices( VID_MICROSOFT, PID_NUI_MOTOR, &pastrDevicePaths, &nCount );
            xnUSBOpenDeviceByPath( *pastrDevicePaths, &dev );
        }

        ~NTiltMotor()
        {
            xnUSBCloseDevice( dev );
            xnUSBFreeDevicesList( pastrDevicePaths );
            xnUSBShutdown();
        }

        void SetAngle( UInt16 angle )
        {
            XnUChar empty[1];
            xnUSBSendControl( dev, XN_USB_CONTROL_TYPE_VENDOR, 0x31,angle * 2, 0, empty, 0, 0 );
        }


    private:
        
        static const UInt32 VID_MICROSOFT = 0x45e;
        static const UInt32 PID_NUI_MOTOR = 0x02b0;

        XN_USB_DEV_HANDLE dev;
        const XnUSBConnectionString* pastrDevicePaths;
    };

	public ref class TiltMotor
	{
    public:

        TiltMotor()
        {
            motor = new NTiltMotor();
        }

        !TiltMotor()
        {
            delete motor;
        }

        ~TiltMotor()
        {
            this->!TiltMotor();
        }

        void SetAngle( UInt16 angle )
        {
            motor->SetAngle( angle );
        }



    private:

        NTiltMotor* motor;
	};
}
