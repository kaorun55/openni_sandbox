#ifndef SKELTONDRAWER_H_INCLUDE
#define SKELTONDRAWER_H_INCLUDE

#include <opencv/cv.h>

#include <XnCppWrapper.h>

class SkeltonDrawer
{
public:
  SkeltonDrawer( IplImage* camera, xn::SkeletonCapability& skelton,
    xn::DepthGenerator& depth, XnUserID player )
    :camera_(camera), skelton_(skelton), depth_(depth), player_(player)
  {
  }

  // スケルトンを描画する
  void draw()
  {
    if (!skelton_.IsTracking(player_)) {
      throw std::runtime_error("トラッキングされていません");
    }

    drawLine(XN_SKEL_HEAD, XN_SKEL_NECK);

    drawLine(XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
    drawLine(XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
    drawLine(XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);

    drawLine(XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
    drawLine(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
    drawLine(XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);

    drawLine(XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
    drawLine(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);

    drawLine(XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
    drawLine(XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
    drawLine(XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);

    drawLine(XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
    drawLine(XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
    drawLine(XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);

    drawLine(XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);
  }

private:
  // スケルトンの線を描画する
  void drawLine(XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
  {
    // 各箇所の座標を取得する
    XnSkeletonJointPosition joint1, joint2;
    skelton_.GetSkeletonJointPosition(player_, eJoint1, joint1);
    skelton_.GetSkeletonJointPosition(player_, eJoint2, joint2);
    if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5) {
      return;
    }

    // 座標を変換する
    XnPoint3D pt[2] = { joint1.position, joint2.position };
    depth_.ConvertRealWorldToProjective(2, pt, pt);
    cvLine(camera_,cvPoint(pt[0].X, pt[0].Y), cvPoint(pt[1].X, pt[1].Y),
      CV_RGB(0, 0, 0),2,CV_AA ,0);
  }

private:

  IplImage* camera_;
  xn::SkeletonCapability& skelton_;
  xn::DepthGenerator& depth_;
  XnUserID player_;
};

#endif // #ifndef SKELTONDRAWER_H_INCLUDE
