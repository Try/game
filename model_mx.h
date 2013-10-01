#ifndef MODEL_MX_H
#define MODEL_MX_H

#include <Tempest/Model>
#include <Tempest/Half>
#include <Tempest/Render>

#include <cstring>
#include <functional>
#include <cstdint>
#include <cmath>

#include "util/ifstream.h"
#include "util/tnloptimize.h"
#include "graphics/material.h"

struct MVertexF {
  float x,y,z,u;
  float nx, ny, v, bx;
  float by,  h; float nz;
  float bz; uint8_t color[4];

  bool operator == ( const MVertexF& v ) const {
    return memcmp( this, &v, sizeof(*this) )==0;
    }

  struct hash{
    size_t operator()( const MVertexF& mv ) const;
    };

  static const Tempest::VertexDeclaration::Declarator& decl();

  private:
    static Tempest::VertexDeclaration::Declarator mkDecl();
  };

struct MVertex {
  Tempest::Half x,y,z,u;
  Tempest::Half nx, ny, v, bx;
  Tempest::Half by,  h; float nz;
  float bz; uint8_t color[4];

  bool operator == ( const MVertex& v ) const {
    return memcmp( this, &v, sizeof(*this) )==0;
    }

  struct hash{
    size_t operator()( const MVertex& mv ) const;
    };

  static const Tempest::VertexDeclaration::Declarator& decl();

  private:
    static Tempest::VertexDeclaration::Declarator mkDecl();
  };

namespace ModelPhysic{
  enum PhysicType{
    Sphere,
    Box
    };
  }

template< class MVertex >
class ModelImpl : public Tempest::Model<MVertex> {
  typedef ModelImpl<MVertex> ThisType;
  public:
    ModelImpl(){
      std::fill( cen, cen+3, 0 );
      std::fill( pbounds[0], pbounds[0]+2, 0 );
      std::fill( pbounds[1], pbounds[1]+2, 0 );
      std::fill( pbounds[2], pbounds[2]+2, 0 );
      r = 0;

      physicType = ModelPhysic::Sphere;
      }

    void loadMX( Tempest::VertexBufferHolder &vboHolder,
                 Tempest::IndexBufferHolder  &iboHolder,
                 const std::string & fname ) {
      ifstream fin( fname.data() );

      char magic[6] = {};
      fin.read( magic, 5 );

      static const std::string modelstr = "Model";
      if( modelstr != magic ){
        return;
        }

      uint16_t ver  = 0,
               size = 0;

      fin.read( (char*)&ver,  sizeof(ver)  );
      fin.read( (char*)&size, sizeof(size) );

      Tempest::Model<Tempest::DefaultVertex>::Raw raw;
      raw.hasIndex = false;
      raw.vertex.resize( size );

      fin.read( (char*)&raw.vertex[0], size*sizeof(Tempest::DefaultVertex) );

      typename Tempest::Model<MVertex>::Raw rawN;
      rawN.vertex.resize( raw.vertex.size() );
      for( size_t i=0; i<rawN.vertex.size(); ++i ){
        MVertex                &v = rawN.vertex[i];
        Tempest::DefaultVertex &d = raw.vertex[i];

        v.x = d.x;
        v.y = d.y;
        v.z = d.z;

        v.u = d.u;
        v.v = d.v;

        v.nx = d.normal[0];
        v.ny = d.normal[1];
        v.nz = d.normal[2];

        std::fill( v.color, v.color+4, 1) ;
        }

      if( rawN.vertex.size()%3==0 ){
        for( size_t i=0; i<rawN.vertex.size(); i+=3 ){
          computeBiNormal( rawN.vertex[i  ],
                           rawN.vertex[i+1],
                           rawN.vertex[i+2] );
          }
        }

      for( size_t r=0; r<rawN.vertex.size(); ++r ){
        cen[0] += rawN.vertex[r].x;
        cen[1] += rawN.vertex[r].y;
        cen[2] += rawN.vertex[r].z;
        }

      for( int r=0; r<3; ++r )
        cen[r] /= rawN.vertex.size();

      TnlOptimize::index( rawN.vertex, rawN.index );
      this->load( vboHolder, iboHolder, rawN.vertex, rawN.index, MVertex::decl() );

      if( ver>=1 ){
        uint16_t groupsSz = 0;
        fin.read( (char*)&groupsSz,  sizeof(groupsSz)  );
        groups.resize( groupsSz );

        for( size_t i=0; i<groupsSz; ++i ){
          uint16_t begin = 0;
          uint16_t sz    = 0;

          fin.read( (char*)&begin, sizeof(begin) );
          fin.read( (char*)&sz,    sizeof(sz)    );

          groups[i].setModelData( this->slice( begin, sz ) );

          if( ver<=1 ){
            for( size_t r=begin; r<begin+sz; ++r ){
              groups[i].cen[0] += rawN.vertex[r].x;
              groups[i].cen[1] += rawN.vertex[r].y;
              groups[i].cen[2] += rawN.vertex[r].z;
              }

            for( int r=0; r<3; ++r )
              groups[i].cen[r] /= sz;

            float maxR = 0, minR = 1000000;
            for( size_t r=begin; r<begin+sz; ++r ){
              float dx = groups[i].cen[0] - rawN.vertex[r].x;
              float dy = groups[i].cen[1] - rawN.vertex[r].y;
              float dz = groups[i].cen[2] - rawN.vertex[r].z;

              float l = sqrt(dx*dx+dy*dy+dz*dz);
              maxR = std::max(maxR, l);
              minR = std::min(minR, l);
              }

            groups[i].r = (maxR*0.7+minR*0.3);
            } else {
            uint16_t type = 0;
            fin.read( (char*)&type,  sizeof(type)  );

            if( type==0 ){
              groups[i].physicType = ModelPhysic::Sphere;
              fin.read( (char*)&groups[i].r,    sizeof(groups[i].r)  );
              fin.read( (char*)&groups[i].cen,  sizeof(groups[i].cen[0])*3  );
              }

            if( type==1 ){
              groups[i].physicType = ModelPhysic::Box;
              fin.read( (char*)&groups[i].pbounds,  sizeof(groups[i].pbounds[0][0])*6  );
              for( int r=0; r<3; ++r ){
                groups[i].cen[r] = 0.5*( groups[i].pbounds[r][0]+groups[i].pbounds[r][1] );
                }
              }
            }
          }
        }

      }

    float cenX() const {
      return cen[0];
      }

    float cenY() const {
      return cen[1];
      }

    float cenZ() const {
      return cen[2];
      }

    float radius() const {
      return r;
      }

    float boxSzX() const {
      return pbounds[0][0] - pbounds[0][1];
      }

    float boxSzY() const {
      return pbounds[1][0] - pbounds[1][1];
      }

    float boxSzZ() const {
      return pbounds[2][0] - pbounds[2][1];
      }

    ModelPhysic::PhysicType physicType;
    std::vector< ThisType > groups;
  private:
    static void computeBiNormal( MVertex & va,
                                 MVertex & vb,
                                 MVertex & vc ) {
      float a[3] = { va.x-vc.x, va.y-vc.y, va.z-vc.z };
      float b[3] = { vb.x-vc.x, vb.y-vc.y, vb.z-vc.z };

      float t1[2] = { va.u-vc.u, va.v-vc.v };
      float t2[2] = { vb.u-vc.u, vb.v-vc.v };

      if( fabs(t2[1]) > 0.00001 ){
        float k = t1[1]/t2[1];
        float m = ( t1[0]-t2[0]*k );

        float u[4] = { a[0]-b[0]*k, a[1]-b[1]*k, a[2]-b[2]*k, 0 };
        for( int i=0; i<3; ++i )
          u[i] /= m;

        MVertex *v[3] = {&va, &vb, &vc};
        for( int i=0; i<3; ++i ){
          v[i]->bx = -u[0];
          v[i]->by = -u[1];
          v[i]->bz = -u[2];
          }
        //float v[3] = {};
        } else {
        float u[4] = { b[0]/t2[0], b[1]/t2[0], b[2]/t2[0], 0 };

        MVertex *v[3] = {&va, &vb, &vc};
        for( int i=0; i<3; ++i ){
          v[i]->bx = -u[0];
          v[i]->by = -u[1];
          v[i]->bz = -u[2];
          }
        }
      }

    void setModelData(const Tempest::Model<MVertex> &md) {
      Tempest::Model<MVertex>::operator = (md);
      }

    float cen[3], r, pbounds[3][2];
  };

class Model {
  public:
    typedef ModelImpl<MVertex>::Raw Raw;
    typedef MVertex Vertex;

    void loadMX( Tempest::VertexBufferHolder &vboHolder,
                 Tempest::IndexBufferHolder  &iboHolder,
                 const std::string & fname ){
      model.loadMX(vboHolder, iboHolder, fname );
      }

    Raw* getRaw() const {
      Raw  *m = new Model::Raw();

      m->index. resize( model.indexes().size()  );
      m->vertex.resize( model.vertexes().size() );

      model.vertexes().get( m->vertex.begin(),
                            m->vertex.end(),
                            0 );
      model.indexes().get( m->index.begin(),
                           m->index.end(),
                           0 );

      return m;
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const std::vector<Vertex>&   buf,
               const std::vector<uint16_t>& index,
               const Tempest::VertexDeclaration::Declarator& decl ){
      model.load( vboHolder, iboHolder, buf, index, decl );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const std::vector<Vertex>&   buf,
               const Tempest::VertexDeclaration::Declarator& decl ){
      model.load( vboHolder, iboHolder, buf, decl );
      }

    void load( Tempest::VertexBufferHolder & vboHolder,
               Tempest::IndexBufferHolder  & iboHolder,
               const Tempest::Model<MVertex>::Raw& buf,
               const Tempest::VertexDeclaration::Declarator& decl ){
      model.load( vboHolder, iboHolder, buf, decl );
      }

    void setTo( GraphicObject & obj ) const {
      obj.setModel(model);
      }

    size_t vboHandle() const{
      return model.vertexes().handle();
      }

    size_t iboHandle() const{
      return model.indexes().handle();
      }

    const Tempest::ModelBounds &bounds() const {
      return model.bounds();
      }

    void render( Tempest::Render &r) const {
      r.draw( model );
      }

    void render( const Tempest::AbstractMaterial &mat,
                 Tempest::Render &r,
                 const Tempest::Matrix4x4 &object,
                 const Tempest::AbstractCamera &camera ) const {
      r.draw(mat, model, object, camera);
      }

    ModelPhysic::PhysicType physicType() const {
      return model.physicType;
      }

    size_t groupsCount() const {
      return model.groups.size();
      }

    Model group( size_t id ) const {
      Model m;
      m.model = model.groups[id];

      return m;
      }
    float cenX() const {
      return model.cenX();
      }

    float cenY() const {
      return model.cenY();
      }

    float cenZ() const {
      return model.cenZ();
      }

    float radius() const {
      return model.radius();
      }

    float boxSzX() const {
      return model.boxSzX();
      }

    float boxSzY() const {
      return model.boxSzY();
      }

    float boxSzZ() const {
      return model.boxSzZ();
      }

    size_t size() const{
      return model.size();
      }
  private:
    ModelImpl<MVertex> model;
  };

#endif // MODEL_MX_H
