#ifndef OK_TOP_DOWN_VIEW_HPP
#define OK_TOP_DOWN_VIEW_HPP

#include "../camera_view.hpp"

class OkCamera;
class OkObject;

/**
 * @brief Top-down view: places the camera straight above a ground rectangle
 *        (the "sector", e.g. the current terrain chunk), looking perpendicularly
 *        down, at the height needed to frame the whole rectangle. Ignores the
 *        mouse. Set the rectangle with setBounds().
 */
class OkTopDownView : public OkCameraView {
public:
  explicit OkTopDownView(OkCamera *camera, float fovDegrees = 60.0f);

  OkCamera *camera() const override { return _camera; }
  void      update(const OkObject &target, float dt) override;

  // The ground rectangle to frame, in world XZ, at elevation groundY.
  void setBounds(float minX, float minZ, float maxX, float maxZ, float groundY);

  // Height needed for a perpendicular camera with this vertical FOV to fit a
  // rectangle of the given XZ size (with a margin). Pure, unit-tested.
  static float computeHeight(float sizeX, float sizeZ, float fovDegrees,
                             float margin);

private:
  OkCamera *_camera;
  float     _fov;
  float     _centerX;
  float     _centerZ;
  float     _sizeX;
  float     _sizeZ;
  float     _groundY;
  float     _margin;
};

#endif  // OK_TOP_DOWN_VIEW_HPP
