#pragma once

#include <string>
#include <glm/glm.hpp>

enum class PrimitiveType {
	SPHERE,
	CUBE,
	NONHIER_SPHERE,
	NONHIER_CUBE,
	MESH
};

class Primitive {
public:
  Primitive(PrimitiveType type);
  virtual ~Primitive();
  std::string typeStr();
  
  PrimitiveType m_type;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3 & pos, double radius);
  virtual ~NonhierSphere();

  glm::vec3 m_pos;
  double m_radius;
};

class Sphere : public NonhierSphere {
public:
  Sphere();
  virtual ~Sphere();
};
