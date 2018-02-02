#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();
	static const int NUM_COLOUR_PICKERS = 8;
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

private:
	void initGrid();
	void initCube();
	void shrinkStack();
	void growStack();
	void reset();
	void resetColours();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object
	
	GLuint m_cube_vao;
	GLuint m_cube_vbo;
	GLuint m_cube_ebo; // Element Buffer Object
	
	Grid* m_grid;

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[NUM_COLOUR_PICKERS][3];
	float m_view_scale;
	float m_view_rotation;
	int m_countdown;
	int colpicker_group;
	int current_col;
	int current_colpicker;
	int current_x;
	int current_z;
	glm::vec2 m_mouse_GL_coordinate;
	bool m_mouseButtonActive;
	bool m_keyDown_shift;
};
