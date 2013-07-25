#ifndef FRUSTUM_H
#define FRUSTUM_H

namespace Tempest {
  class AbstractCamera;
  class Matrix4x4;
  }

class Frustum {
  public:
    Frustum() = default;
    Frustum( const Tempest::AbstractCamera& c );
    Frustum( const Tempest::Matrix4x4& c );

    void fromMatrix( const Tempest::Matrix4x4& c );

    float f[6][4];
  };

#endif // FRUSTUM_H
