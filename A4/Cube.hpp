#pragma once

#include "Mesh.hpp"

#include <glm/glm.hpp>

class NonhierBox : public Mesh {
public:
  NonhierBox(const glm::vec3 & pos, double size);
  virtual ~NonhierBox();

private:
  glm::vec3 m_pos;
  double m_size;
};

class Cube : public NonhierBox {
public:
  Cube();
  virtual ~Cube();
};
