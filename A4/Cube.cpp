#include "Cube.hpp"
#include "Primitive.hpp"

#include <vector>

using namespace std;
using namespace glm;

Cube::Cube() : NonhierBox(vec3(0.0f, 0.0f, 0.0f), 1.0)
{
  m_type = PrimitiveType::CUBE;
}

Cube::~Cube()
{}

NonhierBox::NonhierBox(const vec3 & pos, double size)
  : m_pos(pos), m_size(size)
{
  m_type = PrimitiveType::NONHIER_CUBE;
  
  vector<vec3> verts = {
    m_pos + vec3(0.0f, 0.0f, 0.0f) * (float)m_size,  // 0: Bottom Back Left
    m_pos + vec3(1.0f, 0.0f, 0.0f) * (float)m_size,  // 1: Bottom Back Right
    m_pos + vec3(0.0f, 0.0f, 1.0f) * (float)m_size,  // 2: Bottom Front Left
    m_pos + vec3(1.0f, 0.0f, 1.0f) * (float)m_size,  // 3: Bottom Front Right
    m_pos + vec3(0.0f, 1.0f, 0.0f) * (float)m_size,  // 4: Top Back Left
    m_pos + vec3(1.0f, 1.0f, 0.0f) * (float)m_size,  // 5: Top Back Right
    m_pos + vec3(0.0f, 1.0f, 1.0f) * (float)m_size,  // 6: Top Front Left
    m_pos + vec3(1.0f, 1.0f, 1.0f) * (float)m_size,  // 7: Top Front Right
  };
  
  vector<TriangleIndices> faces = {
    // Front
    TriangleIndices("front0", 3, 6, 2),
    TriangleIndices("front1", 6, 3, 7),
    
    // Back
    TriangleIndices("back0", 1, 4, 5),
    TriangleIndices("back1", 4, 1, 0),
    
    // Left-side
    TriangleIndices("left0", 4, 2, 6),
    TriangleIndices("left1", 2, 4, 0),
    
    // Right-side
    TriangleIndices("right0", 1, 7, 3),
    TriangleIndices("right1", 7, 1, 5),
    
    // Top
    TriangleIndices("top0", 7, 4, 6),
    TriangleIndices("top1", 4, 7, 5),
    
    // Bottom
    TriangleIndices("bottom0", 1, 2, 0),
    TriangleIndices("bottom1", 2, 1, 3)
  };
  
  setTriangles(verts, faces);
}

NonhierBox::~NonhierBox()
{}
