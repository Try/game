#ifndef PHYSICS_H
#define PHYSICS_H

#include <MyGL/Matrix4x4>
//#include "landscape/terrain.h"

class Terrain;

class Physics {
  struct RigidBody;
  struct AnimatedBody;
  struct TerrainBody;

  struct Animated{
      Animated();
      void setPosition( float x, float y, float z );

      bool isValid() const;
    protected:
      AnimatedBody* data;

    friend class Physics;
    };

  public:
    Physics( int tw, int th );
    ~Physics();

    struct Rigid {
        Rigid();

        float x() const;
        float y() const;
        float z() const;

        MyGL::Matrix4x4 transform();
        float diameter() const;

        bool isValid() const;
        void activate();
        bool isActive();
        void setPosition( float x, float y, float z );
      private:
        RigidBody* data;

      friend class Physics;
      };

    void tick();
    void setTerrain( const Terrain & t );

    struct Sphere : public Rigid {
      };

     Sphere createSphere(float x, float y, float z, float r);
     void free( Rigid &s );

     struct Box : public Rigid {
       };
     Box createBox(float x, float y, float z, float sx, float sy, float sz);


     struct AnimatedSphere : Animated {
         using Animated::setPosition;
         using Animated::isValid;

         float diameter() const;

       friend class Physics;
       };

     AnimatedSphere createAnimatedSphere(float x, float y, float z, float r);
     void free( AnimatedSphere & s );

     struct AnimatedBox : Animated {
         using Animated::setPosition;
         using Animated::isValid;

       friend class Physics;
       };
     AnimatedBox createAnimatedBox( float x, float y, float z,
                                    float sx, float sy, float sz );
     void free( AnimatedBox & s );

  private:
    Physics( const Physics &){}

    //void free( AnimatedBody *&b );

    struct Data;
    Data * data;
  };

#endif // PHYSICS_H
