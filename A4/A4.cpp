/* Debug code blocks */
//#define DEBUG_OUTPUT // Debugging output (cout)
#define DEBUG_CHECKERRORS // Run assertions which may exit program on failure
#define DEBUG_DUMMY // Misc. code not meant to ship with final release

#define DEBUG_PROGRESS // Show progress bar
#define PROGRESS_BAR_SIZE 20 // Number of slots in progress bar

// distance from eye to screen (d)
#define DISTANCE_TO_SCREEN 800

// generic floating point epsilon
#define EPSILON 0.0001

// epsilon distance to prevent surfaces from blocking their own shadowrays 
#define SELF_INTERSECT_EPSILON 0.1

// RGB-value representing no color
#define NO_COLOR -1.0

// Max reflection bounces for a ray
#define RAY_TTL 2

#define ENABLE_AMBIENT_LIGHTING
#define ENABLE_DIFFUSE_LIGHTING
#define ENABLE_SPECULAR_LIGHTING
#define ENABLE_REFLECTION_LIGHTING

#define AMBIENT_FACTOR 0.3
#define REFLECTIVITY_EPSILON 0.1

#include "cs488-framework/MathUtils.hpp"
#include "A4.hpp"
#include "PhongMaterial.hpp"
#include "polyroots.hpp"

#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>

using namespace glm;
using namespace std;

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
) {
  /*
  #ifdef DEBUG_DUMMY
    //printHier(*root);
    mat4 matrix = {
      vec4(3.0f, 0.0f, 0.0f, 3.0f),
      vec4(0.0f, 4.0f, 0.0f, 2.0f),
      vec4(0.0f, 0.0f, 5.0f, 2.0f),
      vec4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    matrix = transpose(matrix);
    
    cout << "before" << matrix << endl;
    
    matrix = unscale(matrix);
    cout << "after" << matrix << endl;
    return;
  #endif
  // */

  // 2D array of colors (top-left is (0,0),
  // bottom-right is (image.width() - 1, image.height() - 1))
  vector<vec3> colorBuffer, bgBuffer;
  colorBuffer.resize(image.width() * image.height());
  bgBuffer.resize(image.width() * image.height());
  for (int i = 0; i < image.width() * image.height(); i++) {
    int x = i % image.width();
    int y = i / image.width();
    colorBuffer.at(i) = vec3(NO_COLOR, NO_COLOR, NO_COLOR);
    bgBuffer.at(i) = ambient + 
                     vec3(1.0f, 0.0f, 0.0f) * (image.height() - 1 - y) / (image.height() - 1) +
                     vec3(0.8f, 0.7f, 0.0f) * y / (image.height() - 1) +
                     vec3(0.0f, 0.0f, 0.4f) * x / (image.width() - 1) +
                     - vec3(0.2f, 0.2f, 0.1f) * (image.width() - 1 - x) / (image.width() - 1);
  }
  
  // Raytrace!
  /*
  #ifdef DEBUG_DUMMY
  for (int yk = image.height() - 1 - 380; yk >= image.height() - 1 - 520; yk--) {
    for (int xk = 690; xk < 820; xk++) {
  #endif // */
  for (int yk = image.height() - 1; yk >= 0; yk--) {
    for (int xk = 0; xk < image.width(); xk++) {
      #ifdef DEBUG_DUMMY
        //if (yk != image.height() - 1 - 100 || xk != 72) { continue; }
      #endif
      // Convert pixel k into world coordinates (pWorld)
      vec3 lookfrom = eye;
      vec3 lookat = eye + view;
      vec3 pWorld = dcsToWcs( vec2(xk, yk), degreesToRadians(fovy),
                              image.width(), image.height(),
                              lookfrom, lookat, up);
      /*
      // Print pWorld
      #ifdef DEBUG_OUTPUT        
        cout << pWorld << endl;
      #endif
      // */
      
      Ray ray(lookfrom, pWorld - lookfrom);
      int pix = dcsToIdx( vec2(xk, image.height() - 1 - yk), image.width());
      colorBuffer.at(pix) = raycolor(ray, *root, ambient, lights, RAY_TTL);
      
      #ifdef DEBUG_PROGRESS
        if (pix % (image.width() * image.height() / PROGRESS_BAR_SIZE) == 0) {
          cout << "Progress: [";
          for (int i = 0; i < PROGRESS_BAR_SIZE; i++) {
            if ( i * (image.width() * image.height() / PROGRESS_BAR_SIZE) < 
                 pix) {
              cout << "=";
            }
            else {
              cout << " ";
            }
          }
          cout << "]" << endl;
        }
      #endif
    }
  }

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
		  int idx = dcsToIdx(vec2(x, y), image.width());
		  if ( colorBuffer.at(idx).r != NO_COLOR &&
		       colorBuffer.at(idx).g != NO_COLOR &&
		       colorBuffer.at(idx).b != NO_COLOR) {
		    image(x, y, 0) = colorBuffer.at(idx).r;
		    image(x, y, 1) = colorBuffer.at(idx).g;
		    image(x, y, 2) = colorBuffer.at(idx).b;     
      }
      else { // Show a background
			  /*// Red: increasing from top to bottom
			  image(x, y, 0) = (double)y / h;
			  // Green: increasing from left to right
			  image(x, y, 1) = (double)x / w;
			  // Blue: in lower-left and upper-right corners
			  image(x, y, 2) = ((y < h/2 && x < w/2)
						    || (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;
			  //*/
			  image(x, y, 0) = bgBuffer.at(idx).r;
			  // Green: increasing from left to right
			  image(x, y, 1) = bgBuffer.at(idx).g;
			  // Blue: in lower-left and upper-right corners
			  image(x, y, 2) = bgBuffer.at(idx).b;
		  }
		}
	}
}

// px: x,y coords of pixel in DCS, where (0,0) is bottom-left of screen
vec3 dcsToWcs(vec2 px, double fovy, uint imageWidth, uint imageHeight,
              vec3 lookfrom, vec3 lookat, vec3 up) {
  // Step 1
  double d = DISTANCE_TO_SCREEN; // distance from eye to image
  mat4 T1 =
      translate(mat4(),
                vec3(-(imageWidth / 2.0f), -(imageHeight / 2.0f), d));
  
  // Step 2
  double h = 2.0 * d * tan(fovy / 2.0); // image height in WCS
  mat4 S2 = scale(mat4(),
                  vec3(-(h / imageHeight), h / imageHeight, 1.0f));
  
  // Step 3
  vec3 Vz = (lookat - lookfrom) / length(lookat - lookfrom);
  vec3 Vx = cross(up, Vz) / length(cross(up, Vz));
  vec3 Vy = cross(Vz, Vx);
  mat4 R3 = {
    vec4(Vx.x, Vx.y, Vx.z, 0.0f),
    vec4(Vy.x, Vy.y, Vy.z, 0.0f),
    vec4(Vz.x, Vz.y, Vz.z, 0.0f),
    vec4(0.0f, 0.0f, 0.0f, 1.0f)
  };
  
  /*
  #ifdef DEBUG_OUTPUT
    cout << "Vx" << Vx << endl;
    cout << "Vy" << Vy << endl;
    cout << "Vz" << Vz << endl;
    cout << "R3: " << R3 << endl;
  #endif
  */
  
  // Step 4
  mat4 T4 = translate(mat4(), lookfrom);
  
  // Get pWorld coordinates
  vec4 px3d(px.x, px.y, 0.0f, 1.0f); // pixel with z = 0

  /*
  #ifdef DEBUG_OUTPUT
    cout << "========================" << endl;
    cout << "px: " << px3d << endl;
    cout << "T1: " << T1 << endl;
    cout << "px: " << T1 * px3d << endl;
    cout << "S2: h " << h << ", imageHeight " << imageHeight << S2 << endl;
    cout << "px: " << S2 * T1 * px3d << endl;
    cout << "R3: " << R3 << endl;
    cout << "px: " << R3 * S2 * T1 * px3d << endl;
    cout << "T4: " << T4 << endl;
    cout << "px: " << T4 * R3 * S2 * T1 * px3d << endl;
    //cout << "Mvw:" << T4 * R3 << endl;
  #endif
  // */

  return vec3(T4 * R3 * S2 * T1 * px3d);
}

// Return raycolor (NO_COLOR if ray hits nothing)
vec3 raycolor( const Ray & ray,
               const SceneNode & scene,
               // Lighting parameters
		           const vec3 & ambient,
		           const list<Light *> & lights,
		           uint ttl,
		           const GeometryNode * raycaster,
		           const TriangleVerts * raycasterTriangle) {
	if (ttl <= 0) {
	  #ifdef DEBUG_CHECKERRORS
	    cout << "Cannot call raycolor with ttl of 0" << endl;
	    exit(EXIT_FAILURE);
    #endif
	  return vec3(NO_COLOR, NO_COLOR, NO_COLOR);
	}
	ttl--;

  vec3 color;
  SurfacePoint point;
  if (hit(ray, scene, point, 0, raycaster, raycasterTriangle)) {
    /*
    #ifdef DEBUG_DUMMY
      if (point.geo->m_name == "plane") {
        //cout << "Raycolor of " << name << "==========================" << endl;
        //cout << "intersection " << point.position << endl;
        //cout << "normal " << point.normal << endl;
        color = vec3(1.0f, 0.0f, 0.0f);
        return color;
      }
    #endif // */
        
    #ifdef ENABLE_AMBIENT_LIGHTING
      color = point.ke + ambient * AMBIENT_FACTOR;
    #endif
    
    for (auto itLight = lights.begin(); itLight != lights.end(); itLight++) {
      #ifdef ENABLE_DIFFUSE_LIGHTING
        if (isPositive(length(point.kd))) {
          color += point.kd * directLight( point.position, point.normal,
                                           *(point.geo), point.triangle,
                                           scene, **itLight);
          /*
          #ifdef DEBUG_OUTPUT
            if (point.name[0] == 's') {
              cout << "sphere " << point.name << "=============================" << endl;
              cout << "color: " << color << endl;
              cout << "intersection: " << point.position << endl;
              cout << "normal: " << point.normal << endl;
              cout << "ray dir" << ray.direction << endl;
            }
          #endif // */
        }
      #endif
      
      #ifdef ENABLE_SPECULAR_LIGHTING
        if (isPositive(length(point.ks))) {
          vec3 _specular = specularLight( ray, point.position, point.normal,
                                          point.shininess, *(point.geo),
                                          point.triangle, scene, **itLight);
          color += point.ks * _specular;
        }
      #endif
      
      #ifdef ENABLE_REFLECTION_LIGHTING
        if (point.reflectivity > REFLECTIVITY_EPSILON &&
            isPositive(length(point.ks)) && ttl > 0) {
          color += reflectedLight( ray, point.position, point.normal,
                                   point.reflectivity,
                                   *(point.geo), point.triangle, scene,
                                   **itLight, lights, ambient, ttl);
        }
      #endif
    }
    
    // cap rgb values
    if (color.r >= 1.0f) {
      color.r = 1.0f;
    }
    if (color.g >= 1.0f) {
      color.g = 1.0f;
    }
    if (color.b >= 1.0f) {
      color.b = 1.0f;
    }
    return color;
  }
  else return vec3(NO_COLOR, NO_COLOR, NO_COLOR);
}

// Return true iff some object was hit
// Store intersection point data
bool hit( const Ray & ray, const SceneNode & scene,
          SurfacePoint & out_intersection, double maxRayLength,
          const GeometryNode * raycaster,
          const TriangleVerts * raycasterTriangle) {
  vector<SurfacePoint> intersections;
  stack<mat4> transStack;
  transStack.push(mat4());
  getIntersections( ray, scene, transStack, intersections, raycaster,
                    raycasterTriangle);

  if (intersections.empty()) {
    return false;
  }
  else {  
    // Return the closest intersection (within maxRayLength)
    SurfacePoint nearestIntersection;
    bool isHit = false;
    for (auto it = intersections.begin(); it != intersections.end(); it++) {
      if ( // avoid intersections with self AND
           (length(it->position - ray.origin) > SELF_INTERSECT_EPSILON) &&
             // (is first intersection OR
           ( (! isHit) ||
             // is nearest intersection)
             ( length(it->position - ray.origin) <
               length(nearestIntersection.position - ray.origin))
             )) {
        nearestIntersection = *it;
        isHit = true;
      }
    }
    
    // nothing hit
    if (! isHit) return false;
    
    // If nearest object is too far, ignore it
    if ( isPositive(maxRayLength) &&
         length(nearestIntersection.position - ray.origin) > maxRayLength) {
      return false;
    }
    
    out_intersection = nearestIntersection;
    return true;
  }
}

// Returns all surfaces that the ray goes through (ignoring backfacing polygons)
void getIntersections( const Ray & ray, const SceneNode & node,
                       stack<mat4> & transStack,
                       vector<SurfacePoint> & out_intersections,
                       const GeometryNode * raycaster,
                       const TriangleVerts * raycasterTriangle) {
  transStack.push(transStack.top() * node.trans);

  if (node.m_nodeType == NodeType::GeometryNode) {
    const GeometryNode * geo = static_cast<const GeometryNode *>(&node);
    vector<SurfacePoint> intersections;
    bool isHit = hitGeo( ray, *geo, intersections, transStack.top(), raycaster,
                         raycasterTriangle);
    /*
    #ifdef DEBUG_OUTPUT
      if (geo->m_name == "poly" && raycaster != NULL && raycaster->m_name == "plane") {
        cout << "checking for shadowray block (from plane to poly)" << endl;
        if (isHit) {
          cout << "hit " << endl;
        }
        else {
          cout << "miss " << endl;
        }
      }
    #endif // */
    out_intersections.insert(out_intersections.end(), intersections.begin(), intersections.end());
  }

  for ( const SceneNode * child : node.children) {
    getIntersections( ray, *child, transStack, out_intersections, raycaster,
                      raycasterTriangle);
  }
  transStack.pop();
}

// Return true iff geo is hit
// Store intersection point data
bool hitGeo( const Ray & ray, const GeometryNode & geo,
             vector<SurfacePoint> & out_intersections, const mat4 & trans,
             const GeometryNode * raycaster,
             const TriangleVerts * raycasterTriangle) {
  bool isHit = false;
  mat4 invtrans = inverse(trans);
  vec4 rayOrigin4(ray.origin.x, ray.origin.y, ray.origin.z, 1.0f);
  vec4 rayDir4(ray.direction.x, ray.direction.y, ray.direction.z, 0.0f);
  Ray warpedRay(vec3(homogenize(invtrans * rayOrigin4)), vec3(invtrans * rayDir4));
  switch(geo.m_primitive->m_type) {
    case PrimitiveType::SPHERE:
    case PrimitiveType::NONHIER_SPHERE: {
      if (raycaster != NULL && raycaster->m_nodeId == geo.m_nodeId) {
        return false; 
      }
      isHit = hitNonhierSphere(warpedRay, geo, out_intersections);
      break; }
    case PrimitiveType::CUBE:
    case PrimitiveType::MESH:
    case PrimitiveType::NONHIER_CUBE: {
      if (raycaster != NULL && raycaster->m_nodeId == geo.m_nodeId) {
        return false;
      }
      isHit = hitNonhierCube(warpedRay, geo, out_intersections);
      break; }
  }
  
  if (isHit) {
    for ( auto it = out_intersections.begin(); it != out_intersections.end();
          it++) {
      vec4 pos4(it->position.x, it->position.y, it->position.z, 1.0f);
      vec4 norm4(it->normal.x, it->normal.y, it->normal.z, 0.0f);
      it->position = vec3(homogenize(trans * pos4));
      it->normal = vec3(transpose(invtrans) * norm4);
    }
  }
  
  return isHit;
}

bool hitNonhierCube( const Ray & ray, const GeometryNode & geo,
                     vector<SurfacePoint> & out_intersections,
                     const GeometryNode * raycaster,
                     const TriangleVerts * raycasterTriangle) {
  const Mesh * mesh = static_cast<const Mesh *>(geo.m_primitive);
  
  #ifdef DEBUG_OUTPUT
    //if (geo.m_name == "poly") {
      //cout << "mesh " << geo.m_name << " has triangles:" << mesh->m_triangles.size() << endl;
      //for ( auto itTriangle = mesh->m_triangles.begin();
      //    itTriangle != mesh->m_triangles.end();
      //    itTriangle++) {
        //cout << "triangle " << itTriangle->name << endl;
      //}
    //}
  #endif
  
  for ( auto itTriangle = mesh->m_triangles.begin();
        itTriangle != mesh->m_triangles.end();
        itTriangle++) {  
        
    if ( raycaster != NULL && raycaster->m_nodeId == geo.m_nodeId &&
         raycasterTriangle != NULL && raycasterTriangle == & * itTriangle) {
      #ifdef DEBUG_OUTPUT
        cout << "mesh self-hit" << endl;
      #endif
      continue; // Mesh triangle cannot block it's own ray     
    }
    
    vec3 intersectionPoint;
    if (hitTriangle(ray, *itTriangle, intersectionPoint)) {
      const PhongMaterial * material =
          static_cast<const PhongMaterial *>(geo.m_material);
      vec3 normal = itTriangle->normal;
      if (isPositive(dot(normal, ray.direction))) {
        normal = - normal;
      }
      SurfacePoint intersection( intersectionPoint, normal,
                                 material->m_kd, material->m_ks,
                                 vec3(0.0f, 0.0f, 0.0f),
                                 material->m_shininess,
                                 material->m_reflectivity,
                                 & geo, & * itTriangle);
      intersection.name = geo.m_name + ":" + itTriangle->name;
      out_intersections.push_back(intersection);
      
      #ifdef DEBUG_OUTPUT
        if (geo.m_name == "poly" && raycaster != NULL && raycaster->m_name == "plane") {
          cout << "hit!!!!" << endl;
        }
      #endif
    }
    #ifdef DEBUG_OUTPUT
      if (geo.m_name == "poly" && raycaster != NULL && raycaster->m_name == "plane") {
        cout << "checking for plane-shadowray-to-poly hit..." << endl;
        
      }
    #endif
  }

  return ! out_intersections.empty();
}

bool hitNonhierSphere( const Ray & ray, const GeometryNode & geo,
                       vector<SurfacePoint> & out_intersections) {
  const NonhierSphere * sphere =
      static_cast<const NonhierSphere *>(geo.m_primitive);
  // Parametric equation of ray: p = a + t (b - a)
  vec3 a = ray.origin;
  vec3 b = normalize(ray.direction);
  
  // Implicit equation of sphere: radius r, center c
  vec3 c = sphere->m_pos;
  double r = sphere->m_radius;
  
  // Quadratic equation to solve for t:
  double A = dot(b, b); // * t ^ 2
  double B = 2.0 * dot(b, a - c); // * t
  double C = dot(a - c, a - c) - pow(r, 2);
  
  double roots[2];
  size_t numRoots = quadraticRoots(A, B, C, roots);
  double t;
  
  if (numRoots == 0) {
    /*
    #ifdef DEBUG_OUTPUT
      cout << "======================================" << endl;
      cout << "ray missed sphere " << geo.m_name << endl;
      cout << "c: " << c << ", r: " << r << endl;
      cout << "a: " << a << ", b: " << b << endl;
    #endif // */
    //cout << "no roots" << endl;
    return false;
  }
  else if (numRoots == 1){
    t = roots[0];
    
    if (isNegative(t)) {
      #ifdef DEBUG_OUTPUT
        cout << "sphere root invalid" << endl;
      #endif
      //cout << "singleton root invalid: " << roots[0] << endl;
      return false;
    }
  }
  else { // numRoots == 2
    bool isRoot0Valid = isPositive(roots[0]); //roots[0] >= 0.0;
    bool isRoot1Valid = isPositive(roots[1]); //roots[1] >= 0.0;
    
    if (isRoot0Valid && isRoot1Valid) {
      t = glm::min(roots[0], roots[1]);
    }
    else if (isRoot0Valid) {
      t = roots[0];
    }
    else if (isRoot1Valid) {
      t = roots[1];
    }
    else {
      #ifdef DEBUG_OUTPUT
        cout << "both sphere roots invalid" << endl;
      #endif
      return false;
    }
  }
  
  const PhongMaterial * material =
      static_cast<const PhongMaterial *>(geo.m_material);
  vec3 intersectionPoint = a + t * b; 
  SurfacePoint intersection( intersectionPoint, intersectionPoint - c,
                             material->m_kd, material->m_ks,
                             vec3(0.0f, 0.0f, 0.0f),
                             material->m_shininess,
                             material->m_reflectivity,
                             & geo);
  intersection.name = geo.m_name;
  out_intersections.push_back(intersection);
  
  //*
  #ifdef DEBUG_OUTPUT
    if (geo.m_name == "s1") {
      cout << "Sphere hit ==================================" << endl;
      cout << geo.m_name << ", at " << intersectionPoint << endl;
      cout << "num roots: " << numRoots << endl;
      cout << "roots: " << roots[0];
      if (numRoots == 2) {
        cout << ", " << roots[1] << endl;
      }
      cout << "t " << t << endl;
      cout << "c: " << c << ", r: " << r << endl;
      cout << "a: " << a << ", b: " << b << endl;
    }    
  #endif // */
  //cout << "root found" << endl;
  return true;
}

// Return true iff triangle is hit
// Store intersection point
bool hitTriangle( const Ray & ray, const TriangleVerts & tri,
                  vec3 & out_intersection) {
  #ifdef DEBUG_OUTPUT
    //cout << "checking for triangle hit: " << tri.name << endl;
  #endif
  TriangleVerts triangle = tri;
  if (isPositive(dot(ray.direction, triangle.normal))) {
    triangle.v1 = tri.v3;
    triangle.v2 = tri.v2;
    triangle.v3 = tri.v1;
  }
  // triangle normal is facing ray
  vec3 R = ray.origin - triangle.v1;
  vec3 col0 = triangle.v2 - triangle.v1;
  vec3 col1 = triangle.v3 - triangle.v1;
  vec3 col2 = - ray.direction;
  mat3 coeff = {
    col0, col1, col2
  };
  
  float D = determinant(coeff);
  
  mat3 coeff1 = coeff;
  coeff1[0] = R;
  float D1 = determinant(coeff1);
  
  mat3 coeff2 = coeff;
  coeff2[1] = R;
  float D2 = determinant(coeff2);
  
  mat3 coeff3 = coeff;
  coeff3[2] = R;
  float D3 = determinant(coeff3);
  
  if (isZero(D)) {
    #ifdef DEBUG_OUTPUT
      //cout << "ray parallel to polygon" << endl;
    #endif
  
    return false; // avoid division by zero
  }
  
  float beta = D1 / D;
  float gamma = D2 / D;
  float t = D3 / D;
  
  if ( ! isNegative(beta) && ! isNegative(gamma) &&
       ! isPositive(beta + gamma - 1.0f) && ! isNegative(t)) {
    // Hit!
    /*
    #ifdef DEBUG_CHECKERRORS
      // Assert: both parametric equations yield same point
      vec3 pt0 = ray.origin + t * ray.direction;
      vec3 pt1 = triangle.v1 + beta * col0 + gamma * col1;
      if (length(pt0 - pt1) >= EPSILON) {
        cout << "intersection-points differ:" << endl;
        cout << pt0 << ", " << pt1 << endl;
        cout << "by " << length(pt0 - pt1) << endl;
        cout << triangle.name << endl;
        exit(EXIT_FAILURE);
      }        
    #endif // */
    
    out_intersection = ray.origin + t * ray.direction;
    /*
    #ifdef DEBUG_OUTPUT
      // Print Hits
      cout << "hit: " << out_intersection << " on " << triangle.name << endl;
    #endif
    // */
    return true;
  }

  /*
  #ifdef DEBUG_OUTPUT
    cout << "=======================================" << endl;
    cout << "missed triangle:" << endl;      
    cout << triangle.name << ": " << triangle.v1 << triangle.v2 << triangle.v3 << endl;
    cout << "D " << D << ", D1 " << D1 << ", D2 " << D2 << ", D3 " << D3 << endl;
    cout << "coeff: " << coeff << endl;
    cout << "beta " << beta << ", gamma " << gamma << endl;
    cout << "beta("<< beta <<") + gamma("<< gamma <<")" << endl;
  #endif
  // */
  return false;
}

// Return color from direct lights
vec3 directLight( const vec3 & point, const vec3 & normal,
                  const GeometryNode & geoSurface,
                  const TriangleVerts * triangle,
                  const SceneNode & scene,
                  const Light & light) {
  vec3 color(0.0f, 0.0f, 0.0f);

  Ray shadowRay(point, light.position - point);
  SurfacePoint blockingPoint;
  if ( ! isNegative(dot(normal, shadowRay.direction)) &&
       ! hit( shadowRay, scene, blockingPoint, length(shadowRay.direction),
              & geoSurface, triangle)) {
    float cosTheta = clamp( dot(shadowRay.direction, normal) / 
                            ( length(shadowRay.direction) * 
                              length(normal)),
                            0.0f, 1.0f);
    double att = attenuation(light, length(shadowRay.direction));
    color += light.colour * cosTheta * att;
    
    /*
    #ifdef DEBUG_OUTPUT
      if (geoSurface.m_name == "plane") {
        cout << "direct light! ============================" << endl;
        cout << "color " << color << endl;
        cout << "point " << point << endl;
        cout << "shadowRay orig " << shadowRay.origin << ", dir " << shadowRay.direction << endl;
        cout << "normal " << normal << endl;
        cout << "light position " << light.position << endl;
        cout << "light colour " << light.colour << endl;
        cout << "cosTheta " << cosTheta << endl;
      }
    #endif
    // */
  }
  /*
  #ifdef DEBUG_OUTPUT
    else if (geoSurface.m_name == "plane") {
      // shadowray blocked:
      cout << "===============================================" << endl; 
      if (blockingPoint.geo->m_nodeId == geoSurface.m_nodeId) {
        cout << "geonode blocking itself: " << geoSurface.m_name << endl;
      }
      
      if (blockingPoint.triangle == triangle) {
        cout << "same triangle blocking" << endl;
      }
      
      if (length(blockingPoint.position - point) <= 1.0f) {
        cout << "blocked near ray origin" << endl;
      }
        
      cout << "point " << point << endl;
      cout << "shadowRay orig " << shadowRay.origin << ", dir " << shadowRay.direction << endl;
      cout << "light position " << light.position << endl;
      cout << "cosTheta " << clamp( dot(shadowRay.direction, normal) / 
                            ( length(shadowRay.direction) * 
                              length(normal)),
                            0.0f, 1.0f) << endl;
      cout << "blocking point " << blockingPoint.name << " " << blockingPoint.position << endl;
      cout << "diff: " << length(blockingPoint.position - point) << endl;
    }
  #endif
  // */
  
  /*
  #ifdef DEBUG_OUTPUT
    cout << "direct light " << color << endl;
  #endif
  */
  return color;
}

vec3 specularLight( const Ray & ray, const vec3 & point, const vec3 & normal,
                    const double & shininess, const GeometryNode & geoSurface,
                    const TriangleVerts * triangle,
                    const SceneNode & scene, const Light & light) {
  vec3 color(0.0f, 0.0f, 0.0f);

  Ray shadowRay(point, light.position - point);
  SurfacePoint blockingPoint;
  if ( ! isNegative(dot(normal, shadowRay.direction)) &&
       ! hit( shadowRay, scene, blockingPoint, length(shadowRay.direction),
              & geoSurface, triangle)) {
    vec3 reflectDir = ggReflection(- shadowRay.direction, normal);
    vec3 toEye = - ray.direction;
    
    /*
    #ifdef DEBUG_OUTPUT
      cout << "=======================" << endl;
      cout << "shadowRay " << shadowRay.direction << endl;
      cout << "normal " << normal << endl;
      cout << "reflect " << reflectDir << endl;
      cout << "toEye " << toEye << endl;
    #endif
    // */
    
    double cosTheta = clamp( dot(reflectDir, toEye) / 
                               (length(reflectDir) * length(toEye)),
                             0.0f, 1.0f);
    double att = attenuation(light, length(shadowRay.direction));
    color += light.colour * pow(cosTheta, shininess) * att;
  }
  
  return color;
}

vec3 reflectedLight( const Ray & ray, const vec3 & point, const vec3 & normal,
                     const double reflectivity, const GeometryNode & geoSurface,
                     const TriangleVerts * triangle,
                     const SceneNode & scene, const Light & lightSrc,
                     const list<Light *> & lights, const vec3 & ambient,
                     uint ttl) {
  vec3 color(0.0f, 0.0f, 0.0f);

  Ray shadowRay(point, lightSrc.position - point);
  SurfacePoint blockingPoint;
  if ( ! isNegative(dot(normal, shadowRay.direction)) &&
       ! hit( shadowRay, scene, blockingPoint, length(shadowRay.direction),
              & geoSurface, triangle)) {
    vec3 reflectDir = ggReflection(- shadowRay.direction, normal);
    vec3 toEye = - ray.direction;
    Ray reflectRay(point, reflectDir);
    if (reflectivity < REFLECTIVITY_EPSILON) {
      return color;
    }
    
    vec3 reflectedColor = raycolor( reflectRay, scene, ambient, lights, ttl,
                                    & geoSurface, triangle);
    if (reflectedColor.r == NO_COLOR) {
      reflectedColor = vec3(0.0f, 0.0f, 0.0f);
    }
    color += reflectivity * reflectedColor;
  }
  
  return color;
}

double attenuation(const Light & light, double distToLight) {
  double kc = light.falloff[0]; // Constant
  double kl = light.falloff[1]; // Linear
  double kq = light.falloff[2]; // Quadratic
  return 1.0 / (kc + kl * distToLight + kq * pow(distToLight, 2));
}

// dcs: DCS coords, where (0, 0) is top-left corner
int dcsToIdx(const vec2 & dcs, uint imageWidth) {
  return dcs.x + dcs.y * imageWidth;
}

bool isZero(float f) {
  return glm::abs(f) <= EPSILON;
}

bool isNegative(float f) {
  return ! isZero(f) && f < 0.0f;
}

bool isPositive(float f) {
  return ! isZero(f) && f > 0.0f;
}

vec3 ggReflection(const vec3 & lightToPoint, const vec3 & surfaceNormal) {
  return normalize(lightToPoint) -
         2.0f * ( dot(lightToPoint, surfaceNormal) /
                  (length(lightToPoint) * length(surfaceNormal))) *
         normalize(surfaceNormal);
}

Ray::Ray(vec3 origin, vec3 direction)
  : origin(origin), direction(direction)
{}

SurfacePoint::SurfacePoint()
{}
  
SurfacePoint::SurfacePoint( vec3 position, vec3 normal, vec3 kd, vec3 ks,
                            vec3 ke, double shininess, double reflectivity,
                            const GeometryNode * geo,
                            const TriangleVerts * triangle)
  : position(position), normal(normal), kd(kd), ks(ks), ke(ke),
    shininess(shininess), reflectivity(reflectivity), geo(geo),
    triangle(triangle)
{}

void printHier(const SceneNode & node) {
  cout << "\"" << node.m_name << "\"";
  if (! node.children.empty()) {
    cout << " children:" << node.children.size() << endl;
  }
  else {
    cout << " childless" << endl;
  }
  
  for ( const SceneNode * child : node.children) {
    if (child->m_nodeType == NodeType::GeometryNode) {
      const GeometryNode * geo = static_cast<const GeometryNode *>(child);
      cout << "\"" << node.m_name << "\"" << " parentOf " << "\"" << geo->m_name << "\"";
      switch (geo->m_primitive->m_type) {
        case PrimitiveType::NONHIER_SPHERE: {
          cout << "(nonhier-sphere)"; break; }
        case PrimitiveType::NONHIER_CUBE: {
          cout << "(nonhier-cube)"; break;
        }
        case PrimitiveType::CUBE: {
          cout << "(cube)"; break;
        }
        case PrimitiveType::MESH: {
          const Mesh * mesh = static_cast<const Mesh *>(geo->m_primitive);
          cout << "(mesh:" << mesh->m_triangles.size() << ")";
          break;
        }
        case PrimitiveType::SPHERE: {
          cout << "(sphere)"; break;
        }
      }
      cout << endl;
    }
    printHier(*child);
  }
}

// remove the scaling transformations from a matrix
mat4 unscale(const mat4 & matrix) {
  mat4 unscaledMat;
  unscaledMat[0] = normalize(matrix[0]);
  unscaledMat[1] = normalize(matrix[1]);
  unscaledMat[2] = normalize(matrix[2]);
  unscaledMat[3] = matrix[3];
  return unscaledMat;
}

vec4 homogenize(vec4 v) {
  return v * (1.0f / v.w);
}
