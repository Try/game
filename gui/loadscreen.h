#ifndef LOADSCREEN_H
#define LOADSCREEN_H

#include <Tempest/Surface>
#include <cstdint>

namespace Tempest{
  class LocalTexturesHolder;
  }

class Resource;

class LoadScreen : public Tempest::Surface{
  public:
    LoadScreen( Resource& res, Tempest::LocalTexturesHolder &t );

  protected:
    void paintEvent(Tempest::PaintEvent &e);

    Resource& res;
    Tempest::Texture2d bg;
    uint64_t t;

    int anim;
  };

#endif // LOADSCREEN_H
