#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};

class Gnomon {
public:
	static const size_t NUM_VERTS = 4;
	static constexpr float SCALE = 0.5f;

	Gnomon();
	Gnomon(glm::vec4 xAxis, glm::vec4 yAxis, glm::vec4 zAxis, glm::vec4 origin);
	void reset();
	glm::vec4 getXAxis();
	glm::vec4 getYAxis();
	glm::vec4 getZAxis();
	glm::vec4 getOrigin();
	void setXAxis(glm::vec4 xAxis);
	void setYAxis(glm::vec4 yAxis);
	void setZAxis(glm::vec4 zAxis);
	void setOrigin(glm::vec4 origin);
	void translate(float dx, float dy, float dz);
	void rotate(float rx, float ry, float rz);
	void print();
	
	std::vector<glm::vec4> basis;
	std::vector<glm::vec4> originalBasis;
	glm::mat4 translationMat, rotationMat;
};

class Cube {
public:
	static const size_t NUM_VERTS = 8;

	Cube();
	void reset();
	void translate(float dx, float dy, float dz);
	void rotate(float rx, float ry, float rz);
	void scale(float sx, float sy, float sz);
	
	std::vector<glm::vec4> vertices;
	glm::mat4 scaleMat, rotationMat, translationMat; // M
	glm::mat4 V;
	Gnomon gnomon;
};

class Viewport {
public:
	Viewport();
	void create();
	void update(glm::vec2 ndc);
	glm::vec2 windowToViewport(glm::vec2 w); // convert window NDC to viewport NDC

	std::vector<glm::vec2> vertices;
	bool isNew;
	glm::vec2 newCorner, topLeftCorner, bottomRightCorner;
	float width, height; // width, height in NDC
};

class LineSegment {
public:
	LineSegment(glm::vec4 A, glm::vec4 B);
	void clip(float near, float far);
	void clipEdge(glm::vec4& P, glm::vec4& normal);
	glm::vec4 L(float t, glm::vec4& A, glm::vec4& B); // parametric representation of line
	float l(glm::vec4& Q, glm::vec4& P, glm::vec4& normal); // implicit representation of line
	void clipEdge(glm::vec2& P, glm::vec2& normal);
	glm::vec2 L(float t, glm::vec2& A, glm::vec2& B); // parametric representation of line
	float l(glm::vec2& Q, glm::vec2& P, glm::vec2& normal); // implicit representation of line

	glm::vec4 A, B;
	glm::vec2 a, b;
	bool shouldDraw;
};

class A2 : public CS488Window {
public:
	static constexpr float FOV_MIN = 5.0f;
	static constexpr float FOV_MAX = 160.0f;
	static constexpr float FOV_RATE_MULT = 4.0f;
	static constexpr float NEAREST = 0.01f;

	A2();
	virtual ~A2();
	static glm::mat4 changeOfBasis(Gnomon from, Gnomon to);
	static void printMat4(std::string name, glm::mat4 mat);
	enum MODE { 
		ROTATE_VIEW,		// O
		TRANSLATE_VIEW, 	// N
		PERSPECTIVE,		// P
		ROTATE_MODEL,		// R
		TRANSLATE_MODEL,	// T
		SCALE_MODEL,		// S
		VIEWPORT,			// V
		MODE_SIZE
	};
protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void reset();
	void initLineData();
	void modeRadioBtn(const char* name, MODE mode, int& key, int* group);

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);
	// convert to vec2, convert world to viewport NDC, and clip
	// lines to viewport, then draw
	void viewportClipThenDraw(
		LineSegment& line
	);
	
	void translateModel(float delta);
	void scaleModel(float delta);
	void rotateModel(float delta);
	
	void translateView(float delta);
	void rotateView(float delta);
	void perspective(float delta);
	
	void homogenize(glm::vec4& v);
	glm::vec2 toNDC(glm::vec2 dc);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;
	Cube m_cube;
	glm::vec2 m_mouse_GL_coordinate;

	glm::vec3 m_currentLineColour;
	int m_mode;
	bool m_leftDown;
	bool m_rightDown;
	bool m_middleDown;
	Gnomon m_worldGnomon;
	Gnomon m_viewGnomon;
	float m_fov;
	float m_near;
	float m_far;
	Viewport m_viewport;
};
