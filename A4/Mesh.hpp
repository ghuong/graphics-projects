#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"

struct TriangleIndices
{
  // Points are specified in counter-clock-wise order (w.r.t. the face normal)
	TriangleIndices( size_t pv1, size_t pv2, size_t pv3 );	
	TriangleIndices( std::string name, size_t pv1, size_t pv2, size_t pv3 );

	size_t v1;
	size_t v2;
	size_t v3;
	std::string name;
};

struct TriangleVerts
{
  TriangleVerts( glm::vec3 v1, glm::vec3 v2, glm::vec3 v3 );
  TriangleVerts( std::string name, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3 );

  glm::vec3 v1;
  glm::vec3 v2;
  glm::vec3 v3;
  glm::vec3 normal;
  std::string name;
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );
  Mesh();
  
  void setTriangles( std::vector<glm::vec3> vertices,
                     std::vector<TriangleIndices> faces);
  void setTriangleVerts(bool isOrderedClockwise = true);
  
  std::vector<TriangleVerts> m_triangles;

private:
	std::vector<glm::vec3> m_vertices;
	std::vector<TriangleIndices> m_faces;

  friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
