#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "Mesh.hpp"
#include "Light.hpp"
#include "Image.hpp"

#include <stack>

class Ray {
  public:
    Ray(glm::vec3 origin, glm::vec3 direction);

    glm::vec3 origin;
    glm::vec3 direction; // not normalized
};

// Container of information about a point on a surface
class SurfacePoint {
  public:
    SurfacePoint();    
    SurfacePoint( glm::vec3 position, glm::vec3 normal, glm::vec3 kd,
                  glm::vec3 ks, glm::vec3 ke, double shininess,
                  double reflectivity,
                  const GeometryNode * geo,
                  const TriangleVerts * triangle = NULL);

    glm::vec3 position, normal, kd, ks, ke;
    double shininess, reflectivity;
    std::string name;
    const GeometryNode * geo; // geometry node on which this point lies
    // specific triangle on which this point lies
    const TriangleVerts * triangle;
};

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

glm::vec3 dcsToWcs(glm::vec2 px, double fovy, uint imageWidth, uint imageHeight,
                   glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 up);

glm::vec3 raycolor( const Ray & ray, const SceneNode & scene,
                    const glm::vec3 & ambient,
                    const std::list<Light *> & lights, uint ttl,
                    const GeometryNode * raycaster = NULL,
                    const TriangleVerts * raycasterTriangle = NULL);

bool hit( const Ray & ray, const SceneNode & scene,
          SurfacePoint & out_intersection, double maxRayLength = 0.0,
          const GeometryNode * raycaster = NULL,
          const TriangleVerts * triangle = NULL);

void getIntersections( const Ray & ray, const SceneNode & node,
                       std::stack<glm::mat4> & transStack,
                       std::vector<SurfacePoint> & out_intersections,
                       const GeometryNode * raycaster = NULL,
                       const TriangleVerts * raycasterTriangle = NULL);

bool hitGeo( const Ray & ray, const GeometryNode & geo,
             std::vector<SurfacePoint> & out_intersections,
             const glm::mat4 & trans,
             const GeometryNode * raycaster = NULL,
             const TriangleVerts * raycasterTriangle = NULL);

bool hitNonhierCube( const Ray & ray, const GeometryNode & geo,
                     std::vector<SurfacePoint> & out_intersections,
                     const GeometryNode * raycaster = NULL,
                     const TriangleVerts * raycasterTriangle = NULL);

bool hitNonhierSphere( const Ray & ray, const GeometryNode & geo,
                       std::vector<SurfacePoint> & out_intersection);

bool hitTriangle( const Ray & ray, const TriangleVerts & triangle,
                  glm::vec3 & out_intersection);

glm::vec3 directLight( const glm::vec3 & point,
                       const glm::vec3 & normal,
                       const GeometryNode & geoSurface,
                       const TriangleVerts * triangle,
                       const SceneNode & scene,
                       const Light & light);

glm::vec3 specularLight( const Ray & ray,
                         const glm::vec3 & point,
                         const glm::vec3 & normal,
                         const double & shininess,
                         const GeometryNode & geoSurface,
                         const TriangleVerts * triangle,
                         const SceneNode & scene,
                         const Light & light);

glm::vec3 reflectedLight( const Ray & ray,
                          const glm::vec3 & point,
                          const glm::vec3 & normal,
                          const double shininess,
                          const GeometryNode & geoSurface,
                          const TriangleVerts * triangle,
                          const SceneNode & scene,
                          const Light & lightSrc,
                          const std::list<Light *> & lights,
                          const glm::vec3 & ambient,
                          uint ttl);

glm::vec3 ggReflection( const glm::vec3 & lightToPoint,
                        const glm::vec3 & surfaceNormal);

double attenuation(const Light & light, double distToLight);

int dcsToIdx(const glm::vec2 & dcs, uint imageWidth);

bool isZero(float f);

bool isNegative(float f);

bool isPositive(float f);

void printHier(const SceneNode & node);

glm::mat4 unscale(const glm::mat4 & matrix);

glm::vec4 homogenize(glm::vec4 v);
