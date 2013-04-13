#include "envobject.h"

#include <Tempest/Render>
#include <cmath>

EnvObject::EnvObject( Scene &s ): AbstractGraphicObject(s){
  viewID = -1;
  this->sceneAddObject();
  }

EnvObject::EnvObject(const EnvObject &x): AbstractGraphicObject(x) {
  viewID = -1;
  form    = x.form;
  m       = x.m;
  m_model = x.m_model;
  mat     = x.mat;


  this->sceneAddObject();
  }

EnvObject::~EnvObject() {
  this->sceneDelObject();
  }

EnvObject &EnvObject::operator =(const EnvObject &x) {
  this->sceneDelObject();

  form    = x.form;
  m       = x.m;
  m_model = x.m_model;
  mat     = x.mat;


  this->sceneAddObject();
  return *this;
  }

void EnvObject::setModel(const Model &m){
  this->sceneDelObject();

  m_model = m;
  setTransform( mat );

  this->sceneAddObject();
  }

const Model &EnvObject::model() const {
  return m_model;
  }

const Tempest::Matrix4x4 &EnvObject::transform() const {
  return mat;
  }

void EnvObject::setTransform(const Tempest::Matrix4x4 &im) {
  Tempest::Matrix4x4 old = mat;
  mat = im;

  m.x = mat.at(3,0);
  m.y = mat.at(3,1);
  m.z = mat.at(3,2);

  float l[3];
  for( int i=0; i<3; ++i )
    l[i] = mat.at(i,0)*mat.at(i,0) + mat.at(i,1)*mat.at(i,1) + mat.at(i,2)*mat.at(i,2);

  float ls = std::max(l[0], std::max(l[1], l[2]));
  m.r = sqrt(ls)*bounds().radius()*mat.at(3,3);

  this->onTransformChanged(old);
  }

Tempest::ModelBounds EnvObject::bounds() const {
  return m_model.bounds();
  }

void EnvObject::render( const Tempest::AbstractMaterial &mat,
                        Tempest::Render &r,
                        const Tempest::Matrix4x4 &object,
                        const Tempest::AbstractCamera &camera) const {
  r.draw(mat, m_model, object, camera);
  }

void EnvObject::render(Tempest::Render &r) const {
  r.draw( m_model );
  }

float EnvObject::x() const {
  return m.x;
  }

float EnvObject::y() const {
  return m.y;
  }

float EnvObject::z() const {
  return m.z;
  }

float EnvObject::sizeX() const {
  return 0;
  }

float EnvObject::sizeY() const {
  return 0;
  }

float EnvObject::sizeZ() const {
  return 0;
  }

float EnvObject::radius() const {
  return m.r;
  }
