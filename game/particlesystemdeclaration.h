#ifndef PARTICLESYSTEMDECLARATION_H
#define PARTICLESYSTEMDECLARATION_H

class ParticleSystemDeclaration {
  public:
    ParticleSystemDeclaration();

    struct D{
      float x,y,z;
      float size;
      float r,g,b,a;
      float angle;
      };

    D initMax, initMin;
    D dmax,    dmin;
    D ddmax,   ddmin;
    int density;
    bool hasDD;

    enum Orign{
      Screen,
      XYOrign,
      Planar
      };
    Orign orign;

    void init(D & d , float c = 0);

    static D mix(const D& min, const D& max);
    static float mix( const float min, const float max, float a );
  };

#endif // PARTICLESYSTEMDECLARATION_H
