#define BUFFER_OFFSET(i) ((char *)NULL + (i)) 
#define GET_R_FROM_INT(col) (((col >> 16) & 0xFF) / 255.0f)
#define GET_G_FROM_INT(col) (((col >> 8) & 0xFF) / 255.0f)
#define GET_B_FROM_INT(col) ((col & 0xFF) / 255.0f)
#define RGB_TO_INT(r,g,b) (((int)(r * 255) << 16) + ((int)(g * 255) << 8) + ((int)(b * 255)))

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
static const size_t MAX_HEIGHT = 20;
static const size_t NUM_VERTS_CUBE = 8 * 3;
static const size_t NUM_INDICES_CUBE = 6 * 2 * 3; 
static const size_t NUM_INDICES_FACE = NUM_INDICES_CUBE / 6;
static const size_t NUM_CUBES_GRID = DIM * DIM * MAX_HEIGHT;
static const float MIN_VIEW_SCALE = 0.3f;
static const float MAX_VIEW_SCALE = 6.0f;
static const float VIEW_SCALING_RATE_MULT = 0.1f;
static const float VIEW_ROTATION_RATE_MULT = 1.3f;
static const int COUNTDOWNFROM = 50;
static const int TOP_FACE_OFFSET = 0 * NUM_INDICES_FACE;
static const int BOTTOM_FACE_OFFSET = 1 * NUM_INDICES_FACE;


//----------------------------------------------------------------------------------------
// ConstructorRGB_TO_INT(r,g,b)
A1::A1()
	: current_col( 0 ),
	  current_x( 0 ),
	  current_z( 0 ),
	  current_colpicker( 0 ),
	  m_mouse_GL_coordinate(dvec2(0.0)),
	  m_view_scale( 1.0f ),
	  m_view_rotation( 0.0f ),
	  m_mouseButtonActive( false ),
	  m_keyDown_shift( false ),
	  m_countdown( COUNTDOWNFROM )
{
	resetColours();
	m_grid = new Grid(DIM);
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{
	delete m_grid;
}

void A1::reset()
{
	current_col = 0;
	current_x = 0;
	current_z = 0;
	current_colpicker = 0;
	m_view_scale = 1.0f;
	m_view_rotation = 0.0f;
	resetColours();
	m_grid->reset();
}

void A1::resetColours()
{
	for (int i = 0; i < NUM_COLOUR_PICKERS; ++i) {
		colour[i][0] = 0.0f;
		colour[i][1] = 0.0f;
		colour[i][2] = 0.0f;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initCube();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::normalize(glm::vec3( 0.0f, 1.0f, 0.0f )) );
	cout << glm::normalize(glm::vec3( 0.0f, 1.0f, 0.0f )) << endl;
	proj = glm::perspective( 
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initCube()
{
	// Vertices of a single cube
	float base_verts [NUM_VERTS_CUBE] =
	{
		// Bottom vertices
		0.0f, 0.0f, 0.0f, // 0 // back left
		1.0f, 0.0f, 0.0f, // 1 // back right
		0.0f, 0.0f, 1.0f, // 2 // front left
		1.0f, 0.0f, 1.0f, // 3 // front right
		// Top vertices
		0.0f, 1.0f, 0.0f, // 4 // back left
		1.0f, 1.0f, 0.0f, // 5 // back right
		0.0f, 1.0f, 1.0f, // 6 // front left
		1.0f, 1.0f, 1.0f // 7 // front right
	};

	// Indices of cube's triangles
	short base_indices [NUM_INDICES_CUBE] =
	{
		// Top face triangles indices
		5, 6, 4, // first triangle
		5, 6, 7, // second triangle
		// Bottom face
		1, 2, 0,
		1, 2, 3,
		// Front face
		6, 7, 2,
		6, 7, 3,
		// Back face
		0, 5, 4,
		0, 5, 1,
		// Left face
		0, 6, 4,
		0, 6, 2,
		// Right face
		1, 7, 3, 
		1, 7, 5
	};
	
	float* verts = new float[NUM_VERTS_CUBE * NUM_CUBES_GRID];
	short* indices = new short[NUM_INDICES_CUBE * NUM_CUBES_GRID];
	
	float* cube_verts = new float [NUM_VERTS_CUBE];
	short* cube_indices = new short [NUM_INDICES_CUBE];

	// Populate verts for every cube in the grid
	for (int x = 0; x < DIM; ++x) {
		for (int z = 0; z < DIM; ++z) {
			for (int y = 0; y < MAX_HEIGHT; ++y) {
				int cube_idx = x*DIM*MAX_HEIGHT + z*MAX_HEIGHT + y;
				
				// create vertices for the current cube in the grid
				std::copy(base_verts, base_verts + NUM_VERTS_CUBE, cube_verts);
				for (int i = 0; i < NUM_VERTS_CUBE; i += 3) {
					cube_verts[ i ] += x;
					cube_verts[ i+1 ] += y;
					cube_verts[ i+2 ] += z;
				}
				
				// copy cube verts in
				std::copy(cube_verts, cube_verts + NUM_VERTS_CUBE,
					verts + NUM_VERTS_CUBE * cube_idx);
				
				// create indices for current cube
				std::copy(base_indices, base_indices + NUM_INDICES_CUBE, cube_indices);
				
				for (int i = 0; i < NUM_INDICES_CUBE; ++i) {
					cube_indices[ i ] += NUM_VERTS_CUBE * cube_idx / 3;
				}
				
				// copy element verts in
				std::copy(cube_indices, cube_indices + NUM_INDICES_CUBE,
					indices + NUM_INDICES_CUBE * cube_idx);
			}
		}
	}
	
	delete [] cube_verts;
	delete [] cube_indices;

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the cube vertex buffer object
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, NUM_VERTS_CUBE * NUM_CUBES_GRID * sizeof(float),
		verts, GL_DYNAMIC_DRAW );
	
	// Create the cube element buffer object
	glGenBuffers( 1, &m_cube_ebo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cube_ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, NUM_INDICES_CUBE * NUM_CUBES_GRID * sizeof(short),
		indices, GL_STATIC_DRAW );
		
	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	delete [] verts;
	delete [] indices;

	CHECK_GL_ERRORS;
}

void A1::growStack()
{
	int current_height = m_grid->getHeight( current_x, current_z );
	
	if (current_height == 0) { // new stacks are given the active colour
		m_grid->setColour( current_x, current_z, current_col );
		m_grid->setColourPicker( current_x, current_z, current_colpicker );
	}
	
	if (current_height < MAX_HEIGHT) {
		m_grid->setHeight( current_x, current_z, current_height + 1 );
	}
}

void A1::shrinkStack() {
	int current_height = m_grid->getHeight( current_x, current_z );
	if (current_height > 0) {
		m_grid->setHeight( current_x, current_z, current_height - 1 );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
	m_countdown--;
	if (m_countdown < 0) {
		m_countdown = COUNTDOWNFROM;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		
		if( ImGui::Button( "Reset" ) ) {
			reset();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		for (int i = 0; i < NUM_COLOUR_PICKERS; ++i) {
			ImGui::PushID( i );
			if (ImGui::ColorEdit3( "##Colour", colour[i] )) {
				// find all stacks of this colour, and update them
				int new_colour = RGB_TO_INT(colour[i][0], colour[i][1], colour[i][2]);
				for (int x = 0; x < DIM; ++x) {
					for (int z = 0; z < DIM; ++z) {
						if ( m_grid->getColourPicker( x, z ) == i ) {
							m_grid->setColour( x, z, new_colour );								
						}
					}
				}
				
				// when updating the current colour picker, update current_col
				if ( current_colpicker == i ) {
					current_col = new_colour;
				}
			}
			ImGui::SameLine();
			if( ImGui::RadioButton( "##Col", &colpicker_group, i ) ) {
				// Select this colour.
				current_col = RGB_TO_INT(colour[i][0], colour[i][1], colour[i][2]);
				current_colpicker = i;
				m_grid->setColour( current_x, current_z, current_col );
				m_grid->setColourPicker( current_x, current_z, i );
			}
			ImGui::PopID();
		}

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		vec3 y_axis(0.0f,1.0f,0.0f);
		mat4 camera = translate(view, vec3(0.0f));
		camera *= scale(mat4(), vec3(m_view_scale));
		camera *= rotate(mat4(), m_view_rotation, y_axis);
		
		cout << "camera: " << camera << endl;

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( camera ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		glBindVertexArray( m_cube_vao );
		for (int x = 0; x < DIM; ++x) {
			for (int z = 0; z < DIM; ++z) {
				int col = m_grid->getColour(x, z);
				glUniform3f( col_uni, GET_R_FROM_INT(col), GET_G_FROM_INT(col), GET_B_FROM_INT(col) );
				glDrawElements( GL_TRIANGLES,
								NUM_INDICES_CUBE * m_grid->getHeight(x, z),
								GL_UNSIGNED_SHORT,
								// buffer offset macro is used to compute the offset in the indices buffer
								// the x,z multiplication is to skip past all of the indices before the cube stack
								// at coords x,z
								BUFFER_OFFSET(NUM_INDICES_CUBE*sizeof(short)*(x*DIM*MAX_HEIGHT + z*MAX_HEIGHT)));
			}
		}
		
		// Highlight the active square.
		glDisable( GL_DEPTH_TEST );
		if (m_countdown < COUNTDOWNFROM / 2) {
			int col = m_grid->getColour( current_x, current_z );
			int height = m_grid->getHeight(current_x, current_z);
			glUniform3f( col_uni, GET_R_FROM_INT(col), GET_G_FROM_INT(col), GET_B_FROM_INT(col) );
			glDrawElements( GL_TRIANGLES,
							NUM_INDICES_CUBE * height,
							GL_UNSIGNED_SHORT,
							BUFFER_OFFSET(NUM_INDICES_CUBE*sizeof(short)*(current_x*DIM*MAX_HEIGHT + current_z*MAX_HEIGHT)));
			glUniform3f( col_uni, 1, 1, 1 ); // flash white/black
			if (height > 0) { // highlight top face of cube stack
				glDrawElements( GL_TRIANGLES,
								NUM_INDICES_FACE,
								GL_UNSIGNED_SHORT,
								BUFFER_OFFSET(NUM_INDICES_CUBE*sizeof(short)*
									(current_x*DIM*MAX_HEIGHT + current_z*MAX_HEIGHT + (height - 1)) + sizeof(short)*TOP_FACE_OFFSET));
			}
			else { // highlight bottom face of cube stack
				glDrawElements( GL_TRIANGLES,
								NUM_INDICES_FACE,
								GL_UNSIGNED_SHORT,
								BUFFER_OFFSET(NUM_INDICES_CUBE*sizeof(short)*
									(current_x*DIM*MAX_HEIGHT + current_z*MAX_HEIGHT) + sizeof(short)*BOTTOM_FACE_OFFSET));
			}
		}
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	vec2 old_mouse_coords = m_mouse_GL_coordinate;
	m_mouse_GL_coordinate = vec2 (
			(2.0f * xPos) / m_windowWidth - 1.0f,
			1.0f - ( (2.0f * yPos) / m_windowHeight)
	);
	vec2 delta = m_mouse_GL_coordinate - old_mouse_coords;

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		if (m_mouseButtonActive) {
			float delta_x = delta.x;
			m_view_rotation += delta_x * VIEW_ROTATION_RATE_MULT;		
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);
	
	if (actions == GLFW_PRESS) {
		if (!ImGui::IsMouseHoveringAnyWindow()) {
			m_mouseButtonActive = true;
		}
	}

	if (actions == GLFW_RELEASE) {
		m_mouseButtonActive = false;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	m_view_scale += yOffSet * VIEW_SCALING_RATE_MULT;
	if (m_view_scale > MAX_VIEW_SCALE) {
		m_view_scale = MAX_VIEW_SCALE;
	}
	else if (m_view_scale < MIN_VIEW_SCALE) {
		m_view_scale = MIN_VIEW_SCALE;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if ( key == 'Q' ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if ( key == 'R' ) {
			reset();
		}
		if ( key == GLFW_KEY_SPACE ) {
			growStack();
		}
		if ( key == GLFW_KEY_BACKSPACE ) {
			shrinkStack();
		}
		
		// Arrow keys
		if ( key == GLFW_KEY_UP || key == GLFW_KEY_DOWN ||
			 key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT ) {
			int old_height = m_grid->getHeight( current_x, current_z );
			int old_colour = m_grid->getColour( current_x, current_z );
			int old_colourpicker = m_grid->getColourPicker( current_x, current_z );

			if ( key == GLFW_KEY_UP ) {
				if (current_z > 0) {
					current_z--;
				}
			}
			if ( key == GLFW_KEY_DOWN ) {
				if (current_z < DIM - 1) {
					current_z++;
				}
			}
			if ( key == GLFW_KEY_LEFT ) {
				if (current_x > 0) {
					current_x--;
				}
			}
			if ( key == GLFW_KEY_RIGHT ) {
				if (current_x < DIM - 1) {
					current_x++;
				}
			}
		
			if (m_keyDown_shift) {
				m_grid->setHeight( current_x, current_z, old_height );
				m_grid->setColour( current_x, current_z, old_colour );
				m_grid->setColourPicker( current_x, current_z, old_colourpicker );
			}
		}
		
		if ( key == GLFW_KEY_LEFT_SHIFT ) {
			m_keyDown_shift = true;
		}
	}
	
	if ( action == GLFW_RELEASE ) {
		if ( key == GLFW_KEY_LEFT_SHIFT ) {
			m_keyDown_shift = false;
		}
	}

	return eventHandled;
}
