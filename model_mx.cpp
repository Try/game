#include "model_mx.h"

#include <cstring>
#include <cmath>
#include "util/ifstream.h"

#include "util/tnloptimize.h"

Tempest::VertexDeclaration::Declarator MVertex::mkDecl() {
  Tempest::VertexDeclaration::Declarator d;
  d   .add( Tempest::Decl::float3, Tempest::Usage::Position )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord )
      .add( Tempest::Decl::float3, Tempest::Usage::Normal   )
      .add( Tempest::Decl::float4, Tempest::Usage::Color    )
      .add( Tempest::Decl::float4, Tempest::Usage::BiNormal );

  return d;
  }

const Tempest::VertexDeclaration::Declarator &MVertex::decl() {
  static Tempest::VertexDeclaration::Declarator d = mkDecl();
  return d;
  }

Model::Model() {
  std::fill( cen, cen+3, 0 );
  std::fill( pbounds[0], pbounds[0]+2, 0 );
  std::fill( pbounds[1], pbounds[1]+2, 0 );
  std::fill( pbounds[2], pbounds[2]+2, 0 );
  r = 0;

  physicType = Sphere;
  }

void Model::computeBiNormal( MVertex &va, MVertex &vb, MVertex &vc ) {
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
    /*
    float l = 0;
    for( int i=0; i<3; ++i )
      l += u[i]*u[i];//u[i]*=m;
    l = sqrt(l);
    for( int i=0; i<3; ++i )
      u[i]/= l;
      */

    for( int i=0; i<4; ++i ){
      va.bnormal[i] = -u[i];
      vb.bnormal[i] = -u[i];
      vc.bnormal[i] = -u[i];
      }
    //float v[3] = {};
    } else {
    float u[4] = { b[0]/t2[0], b[1]/t2[0], b[2]/t2[0], 0 };

    for( int i=0; i<4; ++i ){
      va.bnormal[i] = -u[i];
      vb.bnormal[i] = -u[i];
      vc.bnormal[i] = -u[i];
      }
    }
  }

void Model::setModelData(const Tempest::Model<MVertex> &md) {
  Tempest::Model<MVertex>::operator = (md);

  }

void Model::loadMX( Tempest::VertexBufferHolder & vboHolder,
                    Tempest::IndexBufferHolder  & iboHolder,
                    const std::string &fname) {
  ifstream fin( fname.data() );

  char magic[6] = {};
  fin.read( magic, 5 );

  if( std::string(magic)!="Model" ){
    //fin.close();
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

  Raw rawN;
  rawN.vertex.resize( raw.vertex.size() );
  for( size_t i=0; i<rawN.vertex.size(); ++i ){
    MVertex             &v = rawN.vertex[i];
    Tempest::DefaultVertex &d = raw.vertex[i];

    memcpy( &v, &d, sizeof(d) );
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
  load( vboHolder, iboHolder, rawN.vertex, rawN.index, MVertex::decl() );
  //load( vboHolder, iboHolder, rawN, MVertex::decl() );

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
          groups[i].physicType = Sphere;
          fin.read( (char*)&groups[i].r,  sizeof(groups[i].r)  );
          fin.read( (char*)&groups[i].cen,  sizeof(groups[i].cen)*3  );
          }

        if( type==1 ){
          groups[i].physicType = Box;
          fin.read( (char*)&groups[i].pbounds,  sizeof(groups[i].pbounds[0][0])*6  );
          for( int r=0; r<3; ++r ){
            groups[i].cen[r] = 0.5*( groups[i].pbounds[r][0]+groups[i].pbounds[r][1] );
            }
          }
        }
      }
    }

  //fin.close();
  }

float Model::cenX() const {
  return cen[0];
  }

float Model::cenY() const {
  return cen[1];
  }

float Model::cenZ() const {
  return cen[2];
  }

float Model::radius() const {
  return r;
  }

float Model::boxSzX() const {
  return pbounds[0][0] - pbounds[0][1];
  }

float Model::boxSzY() const {
  return pbounds[1][0] - pbounds[1][1];
  }

float Model::boxSzZ() const {
  return pbounds[2][0] - pbounds[2][1];
  }

size_t MVertex::hash::operator ()(const MVertex &mv) const {
  size_t ret = 0;
  char *v  = (char*)&mv;
  char *vx = v+sizeof(*this);

  for(; v!=vx; ++v)
    ret += *v;

  return ret;
  }
