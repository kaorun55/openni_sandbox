#include <iostream>
#include <vector>

#include <XnCppWrapper.h>

void main()
{
    try {
        xn::Context context;
        XnStatus rc = context.Init();
        if ( rc != XN_STATUS_OK ) {
            throw std::runtime_error( xnGetStatusString( rc ) );
        }

        {
            xn::ImageGenerator image;
            rc = image.Create( context );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }

            XnUInt32 count = image.GetSupportedMapOutputModesCount();
            std::vector< XnMapOutputMode > outputmode( count );
            rc = image.GetSupportedMapOutputModes( &outputmode[0], count );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }

            std::cout << "Supported Image Output Mode Count : " << outputmode.size() << std::endl;
            for ( int i = 0; i < outputmode.size(); ++i ) {
                std::cout << outputmode[i].nXRes << ", " << outputmode[i].nYRes << ", " << outputmode[i].nFPS << std::endl;
            }
        }

        {
            xn::DepthGenerator depth;
            rc = depth.Create( context );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }

            XnUInt32 count = depth.GetSupportedMapOutputModesCount();
            std::vector< XnMapOutputMode > outputmode( count );
            rc = depth.GetSupportedMapOutputModes( &outputmode[0], count );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }

            std::cout << "Supported Depth Output Mode Count : " << outputmode.size() << std::endl;
            for ( int i = 0; i < outputmode.size(); ++i ) {
                std::cout << outputmode[i].nXRes << ", " << outputmode[i].nYRes << ", " << outputmode[i].nFPS << std::endl;
            }
        }

        {
            xn::AudioGenerator audio;
            rc = audio.Create( context );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }

            XnUInt32 count = audio.GetSupportedWaveOutputModesCount();
            std::vector< XnWaveOutputMode > outputmode( count );
            rc = audio.GetSupportedWaveOutputModes( &outputmode[0], count );
            if ( rc != XN_STATUS_OK ) {
                throw std::runtime_error( xnGetStatusString( rc ) );
            }

            std::cout << "Supported Wave Output Mode Count : " << outputmode.size() << std::endl;
            for ( int i = 0; i < outputmode.size(); ++i ) {
                std::cout << outputmode[i].nSampleRate << ", " << (XnUInt32)outputmode[i].nChannels << ", " << outputmode[i].nBitsPerSample << std::endl;
            }
        }
    }
    catch ( std::exception& ex ) {
        std::cout << ex.what() << std::endl;
    }
}
