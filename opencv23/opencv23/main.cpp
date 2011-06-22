#include <iostream>

#include <opencv2/opencv.hpp>

void main()
{
  try {
    char* name = "openni";
    cv::namedWindow( name );

    cv::VideoCapture     capture( CV_CAP_OPENNI );
    while ( 1 ) { 
      // データの更新を待つ
      capture.grab(); 

      // RGBを取得して表示
      cv::Mat  rgbImage;
      capture.retrieve( rgbImage, CV_CAP_OPENNI_BGR_IMAGE ); 
      cv::imshow( name, rgbImage );

      if ( cv::waitKey( 10 ) >= 0 ) {
        break; 
      }
    }

    cv::destroyAllWindows();
  }
  catch ( ... ) {
    std::cout << "exception!!" << std::endl;
  }
}
