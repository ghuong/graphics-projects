#include "Primitive.hpp"

using namespace std;
using namespace glm;

Primitive::Primitive(PrimitiveType type) : m_type(type)
{}

Primitive::~Primitive()
{}

string Primitive::typeStr() {
  switch(m_type) {
    case PrimitiveType::SPHERE:
      return "Sphere";
    case PrimitiveType::CUBE:
      return "Cube";
    case PrimitiveType::NONHIER_SPHERE:
      return "Nonhier-Sphere";
    case PrimitiveType::NONHIER_CUBE:
      return "Nonhier-Cube";
    case PrimitiveType::MESH:
      return "Mesh";
    default:
      return "Invalid type";
  }
}

Sphere::Sphere() : NonhierSphere(vec3(0.0f, 0.0f, 0.0f), 1.0)
{
  m_type = PrimitiveType::SPHERE;
}

Sphere::~Sphere()
{}

NonhierSphere::NonhierSphere(const vec3 & pos, double radius)
  : Primitive(PrimitiveType::NONHIER_SPHERE), m_pos(pos), m_radius(radius)
{}

NonhierSphere::~NonhierSphere()
{}

