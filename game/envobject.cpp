#include "envobject.h"

#include <MyGL/Render>

EnvObject::EnvObject( Scene &s ): AbstractGraphicObject(s){
  viewID = -1;
  }

void EnvObject::setModel(const Model &m){
  m_model = m;
  }

const Model &EnvObject::model() const {
  return m_model;
  }

MyGL::Matrix4x4 EnvObject::transform() const {
  return mat;
  }

void EnvObject::setTransform(const MyGL::Matrix4x4 &m) {
  mat = m;
  }

MyGL::ModelBounds EnvObject::bounds() const {
  return m_model.bounds();
  }

void EnvObject::render( const MyGL::AbstractMaterial &mat,
                        MyGL::Render &r,
                        const MyGL::Matrix4x4 &object,
                        const MyGL::AbstractCamera &camera) const {
  r.draw(mat, m_model, object, camera);
  }

void EnvObject::render(MyGL::Render &r) const {
  r.draw( m_model );
  }
