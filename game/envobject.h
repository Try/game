#ifndef ENVOBJECT_H
#define ENVOBJECT_H

#include <MyGL/AbstractGraphicObject>
#include <MyGL/Matrix4x4>

class EnvObject : public MyGL::AbstractGraphicObject {
  public:
    EnvObject( MyGL::Scene & s );

    void setModel( const MyGL::Model<> & m );
    const MyGL::Model<>& model() const;

    MyGL::Matrix4x4 transform() const;
    void setTransform( const MyGL::Matrix4x4 & m );

    MyGL::ModelBounds bounds() const;
    void render(const MyGL::AbstractMaterial &mat,
                MyGL::Render &r,
                const MyGL::Matrix4x4 &object,
                const MyGL::AbstractCamera &camera) const;

  private:
    MyGL::Model<> m_model;
    MyGL::Matrix4x4 mat;
  };

#endif // ENVOBJECT_H
