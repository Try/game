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

    struct Form {
      Physics::Sphere sphere;
      Physics::Box    box;
      } form;

    size_t viewID;
  private:
    Model m_model;
    MyGL::Matrix4x4 mat;
  };

#endif // ENVOBJECT_H
