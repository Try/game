#ifndef GBUFFFER_H
#define GBUFFFER_H

#include <Tempest/Texture2d>

struct GBuffer{
  Tempest::Texture2d color[4];
  Tempest::Texture2d depth;
  };

struct ShadowBuffer{
  Tempest::Texture2d z;
  Tempest::Texture2d depth;
  };

#endif // GBUFFFER_H
