#include <iostream>
#include <fstream>

#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

using namespace std;
using namespace glm;

Mesh::Mesh( const std::string& fname )
	: Primitive(PrimitiveType::MESH)
	, m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( TriangleIndices( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
	
	setTriangleVerts(false);
}

Mesh::Mesh() 
  : Primitive(PrimitiveType::MESH)
{}

void Mesh::setTriangles(vector<vec3> vertices, vector<TriangleIndices> faces) {
  m_vertices = vertices;
  m_faces = faces;
  setTriangleVerts();
}

void Mesh::setTriangleVerts(bool isOrderedCCW) {
  for (auto it = m_faces.begin(); it != m_faces.end(); it++) {
    vec3 v1, v2, v3;
    if (isOrderedCCW) {
      v1 = m_vertices.at(it->v1);
      v2 = m_vertices.at(it->v2);
      v3 = m_vertices.at(it->v3);
    }
    else {
      v1 = m_vertices.at(it->v3);
      v2 = m_vertices.at(it->v2);
      v3 = m_vertices.at(it->v1);
    }
    m_triangles.push_back(TriangleVerts(it->name, v1, v2, v3));
  }
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {" << endl;
  
  for (auto it = mesh.m_triangles.begin(); it != mesh.m_triangles.end(); it++) {
    out << "\ttriangle {" << endl;
    out << "\t\t" << it->v1 << endl; 
    out << "\t\t" << it->v2 << endl; 
    out << "\t\t" << it->v3 << endl; 
    out << "\t\tnormal: " << it->normal << endl;
    out << "\t}" << endl;
  }
  
  /*for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	  if( mesh.m_have_norm ) {
    	  out << " / " << glm::to_string( v.m_normal );
	  }
	  if( mesh.m_have_uv ) {
    	  out << " / " << glm::to_string( v.m_uv );
	  }
  }*/

  out << "}";
  return out;
}

TriangleIndices::TriangleIndices( size_t pv1, size_t pv2, size_t pv3 )
	: v1(pv1), v2(pv2), v3(pv3)
{}

TriangleIndices::TriangleIndices( string name, size_t pv1, size_t pv2,
                                  size_t pv3)
	: v1(pv1), v2(pv2), v3(pv3), name( name )
{}

TriangleVerts::TriangleVerts(vec3 v1, vec3 v2, vec3 v3)
  : v1(v1), v2(v2), v3(v3)
{
  normal = cross(v2 - v1, v3 - v1);
}

TriangleVerts::TriangleVerts(string name, vec3 v1, vec3 v2, vec3 v3)
  : v1(v1), v2(v2), v3(v3), name(name)
{
  normal = cross(v2 - v1, v3 - v1);
}
