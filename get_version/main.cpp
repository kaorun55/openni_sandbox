#include <iostream>
#include <XnCppWrapper.h>

void main()
{
    XnVersion version;
    ::xnGetVersion( &version );
    std::cout << (int)version.nMajor << "." << (int)version.nMinor << "." <<
        version.nMaintenance << "." << version.nBuild << std::endl;

    xn::Version base( 1, 4, 0, 2 );
    if ( base > xn::Version( version ) ) {
        std::cout << "このバージョンでは、ポーズなしキャリブレーションはできません" << std::endl;
    }

    if ( base == xn::Version( version ) ) {
        std::cout << "このバージョンから、ポーズなしキャリブレーションができます" << std::endl;
    }

    if ( base < xn::Version( version ) ) {
        std::cout << "このバージョンは、ポーズなしキャリブレーションができます" << std::endl;
    }
}
