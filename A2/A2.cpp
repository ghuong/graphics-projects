#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <math.h>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;



//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.reserve(kMaxVertices);
	colours.reserve(kMaxVertices);
}

//----------------------------------------------------------------------------------------
// Constructor
Cube::Cube()
{
	reset();
}

//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{
	m_leftDown = false;
	m_rightDown = false;
	m_middleDown = false;
}

//----------------------------------------------------------------------------------------
// Constructor
Gnomon::Gnomon() {
	originalBasis = {
		vec4(1.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 1.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 1.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};

	reset();
}

//----------------------------------------------------------------------------------------
// Constructor
Gnomon::Gnomon(vec4 xAxis, vec4 yAxis, vec4 zAxis, vec4 origin) {
	originalBasis = {
		xAxis,
		yAxis,
		zAxis,
		origin
	};

	reset();
}

//----------------------------------------------------------------------------------------
// Constructor
Viewport::Viewport()
	: isNew (false)
{
}

//----------------------------------------------------------------------------------------
// Constructor
LineSegment::LineSegment(glm::vec4 A, glm::vec4 B)
	: A(A), B(B), shouldDraw(true)
{
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);


	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
	
	reset();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

void A2::reset() {
	m_cube.reset();
	m_worldGnomon.reset();
	m_mode = m_mode = (int)ROTATE_VIEW;
	m_fov = 30.0f;
	m_near = 0.5f;
	m_far = 10.0f;
	
	// view gnomon
	m_viewGnomon = Gnomon(
		vec4(1.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 1.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 1.0f, 0.0f),
		vec4(0.0f, 0.0f, -5.0f, 1.0f));
	
	// initial viewport
	m_viewport.vertices = {
		vec2(2.0f * 0.05f - 1.0f, 1.0f - 2.0f * 0.05f), // 0 // top-left
		vec2(2.0f * 0.95f - 1.0f, 1.0f - 2.0f * 0.05f), // 1 // top-right		
		vec2(2.0f * 0.95f - 1.0f, 1.0f - 2.0f * 0.95f), // 2 // bottom-right
		vec2(2.0f * 0.05f - 1.0f, 1.0f - 2.0f * 0.95f) // 3 // bottom-left
	};
	m_viewport.width = (2.0f * 0.95f - 1.0f) - (2.0f * 0.05f - 1.0f);
	m_viewport.height = (1.0f - 2.0f * 0.05f) - (1.0f - 2.0f * 0.95f);
	m_viewport.topLeftCorner = m_viewport.vertices.at(0);
	m_viewport.bottomRightCorner = m_viewport.vertices.at(2);
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Call at the beginning of frame, before drawing lines:
	initLineData();
	
	// update view gnomon
	m_viewGnomon.setOrigin(m_viewGnomon.translationMat * m_viewGnomon.originalBasis[3]);
	m_viewGnomon.setXAxis(m_viewGnomon.rotationMat * m_viewGnomon.originalBasis[0]);
	m_viewGnomon.setYAxis(m_viewGnomon.rotationMat * m_viewGnomon.originalBasis[1]);
	m_viewGnomon.setZAxis(m_viewGnomon.rotationMat * m_viewGnomon.originalBasis[2]);
	
	// M
	mat4 M = m_cube.translationMat * m_cube.rotationMat * m_cube.scaleMat;
	
	// V ============================================
	// ----------------------------------------------
	vec3 up = vec3(m_viewGnomon.getYAxis());
	vec3 lookat = vec3(m_viewGnomon.getOrigin() + (m_viewGnomon.getZAxis() * 5.0f));
	vec3 lookfrom = vec3(m_viewGnomon.getOrigin());
	vec3 Vz = (lookat - lookfrom) / length(lookat - lookfrom);
	vec3 Vx = cross(up, Vz) / length(cross(up, Vz));
	vec3 Vy = cross(Vz, Vx);
	mat4 R = {
		vec4(Vx.x, Vx.y, Vx.z, 0.0f), 
		vec4(Vy.x, Vy.y, Vy.z, 0.0f), 
		vec4(Vz.x, Vz.y, Vz.z, 0.0f), 
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	R = glm::transpose(R);
	mat4 V = R * glm::translate(mat4(), -lookfrom);	
	// ----------------------------------------------
	// ==============================================
	
	// P ============================================
	// ----------------------------------------------	
	float plusMin = 1.0f; // switch to -1.0f if looking down -z axis
	float theta = m_fov * M_PI / 180.0f; // FOV
	float aspect = 1.0f;
	float cot = 1 / tan(theta/2.0f);
	/*mat4 P = {
		vec4(cot / aspect, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, cot, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, plusMin * (m_far + m_near) / (m_far - m_near), -2.0f * m_far * m_near / (m_far - m_near)),
		vec4(0.0f, 0.0f, plusMin, 0.0f) 
	};*/
	mat4 P = {
		vec4(cot / aspect, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, cot, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, (m_far + m_near) / (m_near), -m_far),
		vec4(0.0f, 0.0f, 1.0f / m_near, 0.0f) 
	};
	P = glm::transpose(P);	
	// ----------------------------------------------
	// ==============================================

	// get verts for cube
	vector<vec4> verts;
	verts.reserve(Cube::NUM_VERTS);
	vector<vec4>::iterator itBegin = m_cube.vertices.begin();
	for (vector<vec4>::iterator it = itBegin; it != m_cube.vertices.end(); it++) {
		vec4 tmp = V * M * (*it);
		verts.push_back(tmp);
	}
	
	// ===================================================
	// Define line segments for cube
	vector<LineSegment> cubeLines;
	
	// bottom face
	cubeLines.push_back(LineSegment(verts[0], verts[1]));
	cubeLines.push_back(LineSegment(verts[0], verts[2]));
	cubeLines.push_back(LineSegment(verts[3], verts[1]));
	cubeLines.push_back(LineSegment(verts[3], verts[2]));
	
	// top face
	cubeLines.push_back(LineSegment(verts[4], verts[5]));
	cubeLines.push_back(LineSegment(verts[4], verts[6]));
	cubeLines.push_back(LineSegment(verts[7], verts[5]));
	cubeLines.push_back(LineSegment(verts[7], verts[6]));
	
	// left-side pillars
	cubeLines.push_back(LineSegment(verts[2], verts[6]));
	cubeLines.push_back(LineSegment(verts[0], verts[4]));
	
	// right-side pillars
	cubeLines.push_back(LineSegment(verts[3], verts[7]));
	cubeLines.push_back(LineSegment(verts[1], verts[5]));
	// ===================================================
	
	for (auto it = cubeLines.begin(); it != cubeLines.end(); it++) {
		// near-far clip each line
		it->clip(m_near, m_far);
		// apply projection and homogenize
		it->A = P * it->A;
		it->B = P * it->B;
		homogenize(it->A);
		homogenize(it->B);
	}
	
	// update cube's gnomon
	m_cube.gnomon.setOrigin(V * m_cube.translationMat * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_cube.gnomon.setXAxis(V * m_cube.rotationMat * vec4(1.0f, 0.0f, 0.0f, 0.0f));
	m_cube.gnomon.setYAxis(V * m_cube.rotationMat * vec4(0.0f, 1.0f, 0.0f, 0.0f));
	m_cube.gnomon.setZAxis(V * m_cube.rotationMat * vec4(0.0f, 0.0f, 1.0f, 0.0f));
	
	// update world gnomon
	m_worldGnomon.setOrigin(V * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_worldGnomon.setXAxis(V * vec4(1.0f, 0.0f, 0.0f, 0.0f));
	m_worldGnomon.setYAxis(V * vec4(0.0f, 1.0f, 0.0f, 0.0f));
	m_worldGnomon.setZAxis(V * vec4(0.0f, 0.0f, 1.0f, 0.0f));
	
	// get verts for gnomons
	vector<vec4> worldGnomonVerts;
	worldGnomonVerts.resize(Gnomon::NUM_VERTS);
	vector<vec4> cubeGnomonVerts;
	cubeGnomonVerts.resize(Gnomon::NUM_VERTS);
	itBegin = m_worldGnomon.basis.begin();
	for (vector<vec4>::iterator it = itBegin; it != m_worldGnomon.basis.end(); it++) {
		int i = it - itBegin;
		worldGnomonVerts.at(i) = *it;
		cubeGnomonVerts.at(i) = m_cube.gnomon.basis.at(i);
		if (i < 3) {
			worldGnomonVerts.at(i) = (worldGnomonVerts.at(i) * 0.5f) + m_worldGnomon.basis.at(3);// * Gnomon::SCALE; // draw shorter vectors
			cubeGnomonVerts.at(i) = (cubeGnomonVerts.at(i) * 0.5f) + m_cube.gnomon.basis.at(3);// * Gnomon::SCALE;
		}
	}
	
	// Define World Gnomon Line Segments ========================
	vector<LineSegment> worldGnomonLines;
	worldGnomonLines.push_back(LineSegment(worldGnomonVerts[3], worldGnomonVerts[0]));
	worldGnomonLines.push_back(LineSegment(worldGnomonVerts[3], worldGnomonVerts[1]));
	worldGnomonLines.push_back(LineSegment(worldGnomonVerts[3], worldGnomonVerts[2]));
	// ===========================================================
	
	for (auto it = worldGnomonLines.begin(); it != worldGnomonLines.end(); it++) {
		// near-far clip each line
		it->clip(m_near, m_far);
		it->A = P * it->A;
		it->B = P * it->B;
		homogenize(it->A);
		homogenize(it->B);
	}
	
	// Define Cube Gnomon Line Segments ========================
	vector<LineSegment> cubeGnomonLines;
	cubeGnomonLines.push_back(LineSegment(cubeGnomonVerts[3], cubeGnomonVerts[0]));
	cubeGnomonLines.push_back(LineSegment(cubeGnomonVerts[3], cubeGnomonVerts[1]));
	cubeGnomonLines.push_back(LineSegment(cubeGnomonVerts[3], cubeGnomonVerts[2]));
	// ===========================================================
	
	for (auto it = cubeGnomonLines.begin(); it != cubeGnomonLines.end(); it++) {
		// near-far clip each line
		it->clip(m_near, m_far);
		it->A = P * it->A;
		it->B = P * it->B;
		homogenize(it->A);
		homogenize(it->B);
	}
	
	// DRAW CUBE ===============================================
	// ----------------------------------------------------
	
	setLineColour(vec3(1.0f, 1.0f, 1.0f)); // white
	int ctr = 0;
	// bottom face
	for (int i = 0; i < 4; i++) {
		viewportClipThenDraw(cubeLines.at(ctr));
		ctr++;
	}
	
	// top face
	setLineColour(vec3(0.0f, 1.0f, 0.0f)); // green top
	for (int i = 0; i < 4; i++) {
		viewportClipThenDraw(cubeLines.at(ctr));
		ctr++;
	}
	
	setLineColour(vec3(1.0f, 1.0f, 1.0f)); // white
	
	// left-side pillars and right-side pillars
	for (int i = 0; i < 4; i++) {
		viewportClipThenDraw(cubeLines.at(ctr));
		ctr++;
	}
	
	// ----------------------------------------------------
	// ====================================================
	
	// world gnomon
	setLineColour(vec3(1.0f, 0.0f, 1.0f)); // magenta
	viewportClipThenDraw(worldGnomonLines.at(0)); // x-axis
	setLineColour(vec3(1.0f, 1.0f, 0.0f)); // yellow
	viewportClipThenDraw(worldGnomonLines.at(1)); // y-axis
	setLineColour(vec3(0.0f, 1.0f, 1.0f)); // cyan
	viewportClipThenDraw(worldGnomonLines.at(2)); // z-axis
	
	// cube gnomon
	setLineColour(vec3(1.0f, 0.0f, 0.0f)); // red
	viewportClipThenDraw(cubeGnomonLines.at(0)); // x-axis
	setLineColour(vec3(0.0f, 1.0f, 0.0f)); // green
	viewportClipThenDraw(cubeGnomonLines.at(1)); // y-axis
	setLineColour(vec3(0.0f, 0.0f, 1.0f)); // blue
	viewportClipThenDraw(cubeGnomonLines.at(2)); // z-axis
	
	// Draw Viewport
	setLineColour(vec3(0.0f, 0.0f, 0.0f)); // black
	drawLine(m_viewport.vertices[0], m_viewport.vertices[1]);
	drawLine(m_viewport.vertices[1], m_viewport.vertices[2]);
	drawLine(m_viewport.vertices[2], m_viewport.vertices[3]);
	drawLine(m_viewport.vertices[3], m_viewport.vertices[0]);
}

void A2::viewportClipThenDraw(LineSegment& line) {
	line.a = m_viewport.windowToViewport(vec2(line.A));
	line.b = m_viewport.windowToViewport(vec2(line.B));
	// clip to viewport!
	vec2 n1(0.0f, -1.0f);
	vec2 n2(1.0f, 0.0f);
	vec2 n3(0.0f, 1.0f);
	vec2 n4(-1.0f, 0.0f);
	line.clipEdge(m_viewport.topLeftCorner, n1);
	line.clipEdge(m_viewport.topLeftCorner, n2);
	line.clipEdge(m_viewport.bottomRightCorner, n3);
	line.clipEdge(m_viewport.bottomRightCorner, n4);

	if (line.shouldDraw) {
		drawLine(line.a, line.b);
	}
}

void LineSegment::clip(float near, float far) {
	vec4 nearP = vec4(0.0f, 0.0f, near, 1.0f);
	vec4 nearNormal = vec4(0.0f, 0.0f, 1.0f, 0.0f);
	vec4 farP = vec4(0.0f, 0.0f, far, 1.0f);
	vec4 farNormal = vec4(0.0f, 0.0f, -1.0f, 0.0f);
	
	clipEdge(nearP, nearNormal);
	clipEdge(farP, farNormal);
}

void LineSegment::clipEdge(vec4& P, vec4& normal) {
	float wecA = l(A, P, normal);
	float wecB = l(B, P, normal);
	if (wecA < 0 && wecB < 0) {
		shouldDraw = false;
		return; // trivially reject
	}
	if (wecA >= 0 && wecB >= 0) {
		return; // trivially accept
	}
	float t = wecA / (wecA - wecB);
	vec4 intersection = L(t, A, B);
	if (wecA < 0) {
		A = intersection; // A + t * (B - A);
	}
	else {
		B = intersection;
	}
}

void LineSegment::clipEdge(vec2& P, vec2& normal) {
	float wecA = l(a, P, normal);
	float wecB = l(b, P, normal);
	if (wecA < 0 && wecB < 0) {
		shouldDraw = false;
		return; // trivially reject
	}
	if (wecA >= 0 && wecB >= 0) {
		return; // trivially accept
	}
	float t = wecA / (wecA - wecB);
	vec2 intersection = L(t, a, b);
	if (wecA < 0) {
		a = intersection; // A + t * (B - A);
	}
	else {
		b = intersection;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		
		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Reset Application" ) ) {
			reset();
		}

		int i = 0;
		modeRadioBtn("R[O]tate View", ROTATE_VIEW, i, &m_mode);
		modeRadioBtn("Tra[N]slate View", TRANSLATE_VIEW, i, &m_mode);
		modeRadioBtn("[P]erspective", PERSPECTIVE, i, &m_mode);
		modeRadioBtn("[R]otate Model", ROTATE_MODEL, i, &m_mode);
		modeRadioBtn("[T]ranslate Model", TRANSLATE_MODEL, i, &m_mode);
		modeRadioBtn("[S]cale Model", SCALE_MODEL, i, &m_mode);
		modeRadioBtn("[V]iewport", VIEWPORT, i, &m_mode);

		ImGui::Text( "Near: %f, Far: %f", m_near, m_far );

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

void A2::modeRadioBtn(const char* name, MODE mode, int& key, int* group) {
	ImGui::PushID( key );
	if( ImGui::RadioButton( name, group, key ) ) {
		m_mode = (int)mode;
	}
	ImGui::PopID();
	key++;
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

void Cube::reset()
{
	vertices = {
		// Bottom vertices
		vec4(-1.0f, -1.0f, -1.0f, 1.0f), // 0 // back left
		vec4(1.0f, -1.0f, -1.0f, 1.0f), // 1 // back right
		vec4(-1.0f, -1.0f, 1.0f, 1.0f), // 2 // front left
		vec4(1.0f, -1.0f, 1.0f, 1.0f), // 3 // front right
		// Top vertices
		vec4(-1.0f, 1.0f, -1.0f, 1.0f), // 4 // back left
		vec4(1.0f, 1.0f, -1.0f, 1.0f), // 5 // back right
		vec4(-1.0f, 1.0f, 1.0f, 1.0f), // 6 // front left
		vec4(1.0f, 1.0f, 1.0f, 1.0f) // 7 // front right
	};
	scaleMat = mat4();
	rotationMat = mat4();
	translationMat = mat4();
	gnomon.reset();
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {http://glm.g-truc.net/0.9.4/api/a00131.html#ga7a31d2864eccfe665409e3b44f5e6e8d
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	vec2 old_mouse_coords = m_mouse_GL_coordinate;
	m_mouse_GL_coordinate = vec2 (
			(2.0f * xPos) / m_windowWidth - 1.0f,
			1.0f - ( (2.0f * yPos) / m_windowHeight)
	);
	vec2 delta = m_mouse_GL_coordinate - old_mouse_coords;

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		switch ((MODE)m_mode) {
			case ROTATE_VIEW: rotateView(delta.x); break;
			case TRANSLATE_VIEW: translateView(delta.x); break;
			case PERSPECTIVE: perspective(delta.x); break;
			case ROTATE_MODEL: rotateModel(delta.x); break;
			case TRANSLATE_MODEL: translateModel(delta.x); break;
			case SCALE_MODEL: scaleModel(delta.x); break;
			case VIEWPORT: if (m_leftDown) { m_viewport.update(m_mouse_GL_coordinate); } break;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (actions == GLFW_PRESS) {
		if (!ImGui::IsMouseHoveringAnyWindow()) {
			switch(button) {
				case GLFW_MOUSE_BUTTON_LEFT: {
					m_leftDown = true;
					if ((MODE)m_mode == VIEWPORT) m_viewport.create();
					break;
				}
				case GLFW_MOUSE_BUTTON_RIGHT: m_rightDown = true; break;
				case GLFW_MOUSE_BUTTON_MIDDLE: m_middleDown = true; break;
			}
		}
	}

	if (actions == GLFW_RELEASE) {
		switch(button) {
			case GLFW_MOUSE_BUTTON_LEFT: m_leftDown = false; break;
			case GLFW_MOUSE_BUTTON_RIGHT: m_rightDown = false; break;
			case GLFW_MOUSE_BUTTON_MIDDLE: m_middleDown = false; break;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	m_windowWidth = width;
	m_windowHeight = height;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if ( key == 'Q' ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		
		if ( key == 'A' ) {
			reset();
		}
		
		switch(key) {
			case 'O': m_mode = (int)ROTATE_VIEW; break;
			case 'N': m_mode = (int)TRANSLATE_VIEW; break;
			case 'P': m_mode = (int)PERSPECTIVE; break;
			case 'R': m_mode = (int)ROTATE_MODEL; break;
			case 'T': m_mode = (int)TRANSLATE_MODEL; break;
			case 'S': m_mode = (int)SCALE_MODEL; break;
			case 'V': m_mode = (int)VIEWPORT; break;
		}
	}

	return eventHandled;
}

void A2::translateModel(float delta) {
	if (m_leftDown) m_cube.translate(delta, 0.0f, 0.0f);
	if (m_middleDown) m_cube.translate(0.0f, delta, 0.0f);
	if (m_rightDown) m_cube.translate(0.0f, 0.0f, delta);
}

void A2::scaleModel(float delta) {
	if (m_leftDown) m_cube.scale(delta, 0.0f, 0.0f);
	if (m_middleDown) m_cube.scale(0.0f, delta, 0.0f);
	if (m_rightDown) m_cube.scale(0.0f, 0.0f, delta);	
}

void A2::rotateModel(float delta) {
	if (m_leftDown) m_cube.rotate(delta, 0.0f, 0.0f);
	if (m_middleDown) m_cube.rotate(0.0f, delta, 0.0f);
	if (m_rightDown) m_cube.rotate(0.0f, 0.0f, delta);	
}

void A2::translateView(float delta) {
	if (m_leftDown) m_viewGnomon.translate(delta, 0.0f, 0.0f);
	if (m_middleDown) m_viewGnomon.translate(0.0f, delta, 0.0f);
	if (m_rightDown) m_viewGnomon.translate(0.0f, 0.0f, delta);	
}

void A2::rotateView(float delta) {
	if (m_leftDown) m_viewGnomon.rotate(delta, 0.0f, 0.0f);
	if (m_middleDown) m_viewGnomon.rotate(0.0f, delta, 0.0f);
	if (m_rightDown) m_viewGnomon.rotate(0.0f, 0.0f, delta);		
}

void A2::perspective(float delta) {
	if (m_leftDown) m_fov += delta * FOV_RATE_MULT;
	
	if (m_middleDown) {
		m_near += delta;
		if (m_near > m_far) {
			m_near = m_far;
		}
	}
	
	if (m_rightDown) {
		m_far += delta;
		if (m_near > m_far) {
			m_far = m_near;
		}
	}
	
	if (m_fov > FOV_MAX) {
		m_fov = FOV_MAX;
	}
	else if (m_fov < FOV_MIN) {
		m_fov = FOV_MIN;
	}
	
	if (m_near < NEAREST) {
		m_near = NEAREST; 
	}
}

vec4 LineSegment::L(float t, vec4& A, vec4& B) {
	return A + t *(B - A);
}

float LineSegment::l(vec4& Q, vec4& P, vec4& normal) {
	return dot((Q - P), normal);
}

vec2 LineSegment::L(float t, vec2& a, vec2& b) {
	return a + t *(b - a);
}

float LineSegment::l(vec2& Q, vec2& P, vec2& normal) {
	return dot((Q - P), normal);
}

void A2::homogenize(vec4& v) {
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;
	v.w /= v.w;
}

void Gnomon::translate(float dx, float dy, float dz) {
	vec4 trans(dx, dy, dz, 0.0f);
	trans = rotationMat * trans;
	
	translationMat = glm::translate(translationMat, vec3(trans));
}

void Gnomon::rotate(float rx, float ry, float rz) {
	rotationMat = glm::rotate(rotationMat, rx, vec3(originalBasis[0]));
	rotationMat = glm::rotate(rotationMat, ry, vec3(originalBasis[1]));
	rotationMat = glm::rotate(rotationMat, rz, vec3(originalBasis[2]));
}

void Cube::translate(float dx, float dy, float dz) {
	vec4 trans(dx, dy, dz, 0.0f);
	trans = rotationMat * trans;
	
	translationMat = glm::translate(translationMat, vec3(trans));
}

void Cube::scale(float sx, float sy, float sz) {
	vec4 scale(sx + 1.0f, sy + 1.0f, sz + 1.0f, 0.0f);
	vec3 scaleXyz(scale.x, scale.y, scale.z);
	
	scaleMat = glm::scale(scaleMat, scaleXyz);
}

void Cube::rotate(float rx, float ry, float rz) {
	rotationMat = glm::rotate(rotationMat, rx, vec3(1.0f, 0.0f, 0.0f));
	rotationMat = glm::rotate(rotationMat, ry, vec3(0.0f, 1.0f, 0.0f));
	rotationMat = glm::rotate(rotationMat, rz, vec3(0.0f, 0.0f, 1.0f));
}

void A2::printMat4(string name, glm::mat4 mat) {
	cout << name << mat << endl;
}

mat4 A2::changeOfBasis(Gnomon from, Gnomon to) {
	mat4 cob;
	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			cob[c][r] = dot(from.basis[r], to.basis[c]);
		}
	}
	
	for (int r = 0; r < 3; r++) {
		cob[3][r] = dot(from.getOrigin() - to.getOrigin(), to.basis[r]);
	}
	
	return cob;
}

vec2 A2::toNDC(vec2 dc) {
	return vec2((2.0f * dc.x) / m_windowWidth - 1.0f,
				1.0f - ( (2.0f * dc.y) / m_windowHeight));
}

void Viewport::create() {
	isNew = true;
}

void Viewport::update(glm::vec2 ndc) {
	if (isNew) {
		newCorner = ndc; // set first corner
		isNew = false;
	}
	else {
		// bound box to the inside:
		ndc.x = glm::max(glm::min(ndc.x, 1.0f), -1.0f);
		ndc.y = glm::max(glm::min(ndc.y, 1.0f), -1.0f);
		newCorner.x = glm::max(glm::min(newCorner.x, 1.0f), -1.0f);
		newCorner.y = glm::max(glm::min(newCorner.y, 1.0f), -1.0f);
		// set all corners
		vertices.at(0) = newCorner;
		vertices.at(1) = vec2(newCorner.x, ndc.y);
		vertices.at(2) = ndc;
		vertices.at(3) = vec2(ndc.x, newCorner.y);
		
		// update width/height
		width = glm::abs(newCorner.x - ndc.x);
		height = glm::abs(newCorner.y - ndc.y);
		
		topLeftCorner = vec2(glm::min(newCorner.x, ndc.x), glm::max(newCorner.y, ndc.y));
		bottomRightCorner = vec2(glm::max(newCorner.x, ndc.x), glm::min(newCorner.y, ndc.y));
	}
}

vec2 Viewport::windowToViewport(vec2 w) {
	return vec2(((w.x + 1.0f) * width / 2.0f) + topLeftCorner.x,
				-((1.0f - w.y) * height / 2.0f) + topLeftCorner.y);
}

void Gnomon::print() {
	cout << "origin: " << basis[3] << endl;
	cout << "x-axis: " << basis[0] << endl;
	cout << "y-axis: " << basis[1] << endl;
	cout << "z-axis: " << basis[2] << endl;
}

void Gnomon::reset() {
	basis = originalBasis;
}

glm::vec4 Gnomon::getXAxis() {
	return basis[0];
}

glm::vec4 Gnomon::getYAxis() {
	return basis[1];
}

glm::vec4 Gnomon::getZAxis() {
	return basis[2];
}

glm::vec4 Gnomon::getOrigin() {
	return basis[3];
}

void Gnomon::setXAxis(glm::vec4 xAxis) {
	basis[0] = xAxis;
}

void Gnomon::setYAxis(glm::vec4 yAxis) {
	basis[1] = yAxis;
}

void Gnomon::setZAxis(glm::vec4 zAxis) {
	basis[2] = zAxis;
}

void Gnomon::setOrigin(glm::vec4 origin) {
	basis[3] = origin;
}
