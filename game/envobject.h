#ifndef ENVOBJECT_H
#define ENVOBJECT_H

#include <MyGL/AbstractGraphicObject>
#include <MyGL/Matrix4x4>

#include "resource.h"

#include "physics/physics.h"
#include "graphics/material.h"

class EnvObject : public AbstractGraphicObject {
  public:
    EnvObject( Scene & s );
    EnvObject( const EnvObject& obj );
    ~EnvObject();

    EnvObject& operator = ( const EnvObject& );

    void setModel( const Model & m );
    const Model& model() const;

    MyGL::Matrix4x4 transform() const;
    void setTransform( const MyGL::Matrix4x4 & m );

    MyGL::ModelBounds bounds() const;

    void render(const MyGL::AbstractMaterial &mat,
                MyGL::Render &r,
                const MyGL::Matrix4x4 &object,
                const MyGL::AbstractCamera &camera) const;

    void render(  MyGL::Render &r ) const;

    float x() const;
    float y() const;
    float z() const;

    float sizeX() const;
    float sizeY() const;
    float sizeZ() const;

    float radius() const;

    struct Form {
      Physics::Sphere sphere;
      Physics::Box    box;
      } form;

    size_t viewID;
  private:
    Model m_model;
    MyGL::Matrix4x4 mat;

    struct {
      float x,y,z,  r;
      } m;
  };

#endif // ENVOBJECT_H
