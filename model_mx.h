#ifndef MODEL_MX_H
#define MODEL_MX_H

#include <Tempest/Model>
#include <cstring>
#include <functional>

struct MVertex {
  float x,y,z;
  float u,v;
  float normal[3];
  float color[4];

  float bnormal[4];

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

class Model : public Tempest::Model<MVertex> {
  public:
    Model();

    void loadMX( Tempest::VertexBufferHolder &vboHolder,
                 Tempest::IndexBufferHolder &iboHolder,
                 const std::string & f );

    std::vector< Model > groups;

    float cenX() const;
    float cenY() const;
    float cenZ() const;

    float radius() const;

    enum PhysicType{
      Sphere,
      Box
      } physicType;


    float boxSzX() const;
    float boxSzY() const;
    float boxSzZ() const;
  private:
    static void computeBiNormal( MVertex & a,
                                 MVertex & b,
                                 MVertex & c );

    void setModelData(const Tempest::Model<MVertex>& md);

    float cen[3], r, pbounds[3][2];
  };

#endif // MODEL_MX_H
