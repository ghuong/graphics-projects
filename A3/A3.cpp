#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  m_mode((int)POSITION_ORIENTATION),
	  m_circle(false),
	  m_zbuffer(true),
	  m_backface_culling(false),
	  m_frontface_culling(false),
	  m_mouseLeftDown(false),
	  m_mouseRightDown(false),
	  m_mouseMiddleDown(false),
	  m_isPicking(false)
{
  resetTimer(m_highlight_timer, 2.0f / HIGHLIGHT_INTERVAL);
  m_undoWarnTimer.timer = 0.0f;
  m_redoWarnTimer.timer = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();
	
	initGeometryJointMap();

  initTransformStack();
	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	SceneNode * rootnode = import_lua(filename);
	
	if (!rootnode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
	
	// attach two additional nodes to the front of the rootnode to store
	// translations and rotations of the puppet (w/ respect to the world)
	SceneNode * translationNode = new SceneNode("translation");
	SceneNode * duplicateRootNode = new SceneNode("newroot");
	SceneNode * rotationNode = new SceneNode("rotation");
	SceneNode * inverseRootNode = new SceneNode("inverseroot");
	translationNode->add_child(duplicateRootNode);
	duplicateRootNode->add_child(rotationNode);
	rotationNode->add_child(inverseRootNode);
	inverseRootNode->add_child(rootnode);	
	m_rootNode = shared_ptr<SceneNode>(translationNode);
	
	duplicateRootNode->trans = rootnode->trans;
	inverseRootNode->trans = glm::inverse(rootnode->trans);
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(getAssetFilePath("VertexShader.vs").c_str());
	m_shader.attachFragmentShader(getAssetFilePath("FragmentShader.fs").c_str());
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader(
	  getAssetFilePath("arc_VertexShader.vs").c_str());
	m_shader_arcCircle.attachFragmentShader(
	  getAssetFilePath("arc_FragmentShader.fs").c_str());
	m_shader_arcCircle.link();
	
	m_shader_picking.generateProgramObject();
	m_shader_picking.attachVertexShader(
	  getAssetFilePath("PickingVertexShader.vs").c_str());
	m_shader_picking.attachFragmentShader(
	  getAssetFilePath("PickingFragmentShader.fs").c_str());
	m_shader_picking.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

    // Enable the vertex shader attribute location for "position" when picking
    m_pickingPositionAttribLocation =
      m_shader_picking.getAttribLocation("position");
    glEnableVertexAttribArray(m_pickingPositionAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::initGeometryJointMap() {
  initGeometryJointMapHelper(*m_rootNode);  
}

//----------------------------------------------------------------------------------------
void A3::initGeometryJointMapHelper(const SceneNode & node) {
  for (const SceneNode * child : node.children) {
    if (node.m_nodeType == NodeType::JointNode &&
        child->m_nodeType == NodeType::GeometryNode) {
      m_geometryJointMap[child->m_nodeId] = node.m_nodeId;
    }
    
    // set the head joint id
    if (node.m_nodeType == NodeType::JointNode &&
        node.m_name == HEAD_JOINT_NAME) {
      m_headJointId = node.m_nodeId;
    }

    initGeometryJointMapHelper(*child);
	}
}
//----------------------------------------------------------------------------------------
void A3::initTransformStack() {
  m_transformStackTop = 0;
  initTransformStackHelper(*m_rootNode);
  pushActiveTransformation();
}

void A3::initTransformStackHelper(const SceneNode & node) {
  if (node.m_nodeType == NodeType::JointNode) {
    const JointNode * joint = dynamic_cast<const JointNode *>(& node);
    JointRotation jr;
    jr.xRange = joint->m_joint_x;
    jr.yRange = joint->m_joint_y;
    jr.x = jr.xRange.init;
    jr.y = jr.yRange.init;
    
    cout << "initial J.R. " << node << ": " << jr.x << "," << jr.y << endl;
    
    m_activeTransformation[node.m_nodeId] = jr;
  }

  for (const SceneNode * child : node.children) {
    initTransformStackHelper(*child);
	}
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
	
	m_shader_picking.enable();
	{
	  //-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader_picking.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;
	
	  //-- Set picking color uniform
	  {
	    m_picking_col_uni =
	      m_shader_picking.getUniformLocation("pickingColor");
	    CHECK_GL_ERRORS;
    }
	}
	m_shader_picking.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...
  tickTimer(m_highlight_timer);
	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	windowFlags |= ImGuiWindowFlags_MenuBar;
	
	float opacity(0.5f);

  ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

    // Menu
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Application")) {
        showApplicationMenu();
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit")) {
        showEditMenu();
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Options")) {
        showOptionsMenu();
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
    
    // Radio Buttons
    int key = 0;
    ImGui::PushID( key );
    if(ImGui::RadioButton( "Position/Orientation (P)", &m_mode, key++))
	    m_mode = (int)POSITION_ORIENTATION;
	  ImGui::PopID();
	  ImGui::PushID( key );
    if(ImGui::RadioButton( "Joints (J)", &m_mode, key++))
	    m_mode = (int)JOINTS;
	  ImGui::PopID();

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

    string undoRedoWarning = "";
    if (m_undoWarnTimer.timer > 0.0f) {
      undoRedoWarning += UNDO_WARNING;
      tickTimer(m_undoWarnTimer);
    }
		
		if (m_redoWarnTimer.timer > 0.0f) {
      undoRedoWarning += REDO_WARNING;
      tickTimer(m_redoWarnTimer);    
    }
		
		ImGui::Text( "%s", undoRedoWarning.c_str() );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A3::showApplicationMenu() {
  if (ImGui::MenuItem("Reset Position", "I")) resetPosition();
  if (ImGui::MenuItem("Reset Orientation", "O")) resetOrientation();
  if (ImGui::MenuItem("Reset Joints", "N")) resetJoints();
  if (ImGui::MenuItem("Reset All", "A")) resetAll();
  if (ImGui::MenuItem("Quit", "Q")) quit();
}

//----------------------------------------------------------------------------------------
void A3::showEditMenu() {
  if (ImGui::MenuItem("Undo", "U")) undo();
  if (ImGui::MenuItem("Redo", "R")) redo();
}

//----------------------------------------------------------------------------------------
void A3::showOptionsMenu() {
  if (ImGui::MenuItem("Circle", "C", &m_circle));
  if (ImGui::MenuItem("Z-Buffer", "Z", &m_zbuffer));
  if (ImGui::MenuItem("Backface Culling", "B", &m_backface_culling));
  if (ImGui::MenuItem("Frontface Culling", "F", &m_frontface_culling));
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix,
		bool isPicking,
		bool isHighlighted,
		float highlightCoefficient,
		float highlightStrength
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modelMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

    if (! isPicking) {
		  //-- Set NormMatrix:
		  location = shader.getUniformLocation("NormalMatrix");
		  mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		  glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		  CHECK_GL_ERRORS;

		  //-- Set Material values:
	    location = shader.getUniformLocation("material.kd");
	    vec3 kd = node.material.kd;
	    if (isHighlighted) {
	      vec3 diffKd = vec3(1.0f, 1.0f, 1.0f) - kd;
	      diffKd = 0.7f * diffKd;
	      diffKd = diffKd * highlightCoefficient;
	      kd = kd + diffKd;
	    }
	    glUniform3fv(location, 1, value_ptr(kd));
	    CHECK_GL_ERRORS;
	    location = shader.getUniformLocation("material.ks");
	    vec3 ks = node.material.ks;
	    if (isHighlighted) {
	      vec3 diffKs = vec3(1.0f, 1.0f, 1.0f) - ks;
	      diffKs = highlightStrength * diffKs;
	      diffKs = diffKs * highlightCoefficient;
	      ks = ks + diffKs;
	    }
	    glUniform3fv(location, 1, value_ptr(ks));
	    CHECK_GL_ERRORS;
	    location = shader.getUniformLocation("material.shininess");
	    glUniform1f(location, node.material.shininess);
	    CHECK_GL_ERRORS;
	  }
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
  if (m_zbuffer) {
	  glEnable( GL_DEPTH_TEST );
	}

  if (m_isPicking) {
    picking(*m_rootNode);
    m_isPicking = false;
  }
  
  renderSceneGraph(*m_rootNode); // render normally
	
	if (glIsEnabled(GL_DEPTH_TEST)) {
    glDisable( GL_DEPTH_TEST );
  }

  if (m_circle) {
	  renderArcCircle();
	}
}

void A3::picking(const SceneNode & root) {
  glClearColor(1.0, 1.0, 1.0, 1.0); // set all color buffers to white
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear screen
  
  renderSceneGraph(*m_rootNode, true); // render scene graph in pick mode
  // tell openGL to send all pending commands to GPU
  glFlush();  
  // wait until everything is really drawn
  glFinish();  
  // Configure how glReadPixels will behave w.r.t. memory alignment
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  unsigned char data[4];
  // read pixels under mouse
  glReadPixels(m_raw_mouse_coords.x, m_raw_mouse_coords.y, 1, 1, GL_RGBA,
               GL_UNSIGNED_BYTE, data);
  int pickedId = colorToId(data);
  
  if (pickedId != 0x00FFFFFF) { // didn't click the background
    //cout << "picked " << pickedId << endl;
    auto jointIter = m_geometryJointMap.find(pickedId);
    if (jointIter == m_geometryJointMap.end()) {
      cout << "picked geoNode " << pickedId << " which has no associated joint" << endl;
    }
    else {
      cout << "picked joint " << m_geometryJointMap[pickedId] << endl;
      toggleJointSelection(m_geometryJointMap[pickedId]);
    }
  }
  else {
    cout << "picked nothing" << endl;
  }

  glClearColor(0.35, 0.35, 0.35, 1.0); // reset background color
  // clear screen for real rendering
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root, bool isPicking) {

  if (m_backface_culling && m_frontface_culling) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT_AND_BACK);
  }
  else if (m_backface_culling) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
  }
  else if (m_frontface_culling) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
  }
  else {
    glDisable(GL_CULL_FACE);
  }

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

  //cout << "rendering root... -------------------------------------------" << endl;
  stack<mat4> matStack;
  matStack.push(mat4());
  renderSceneNode(root, matStack, isPicking);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::renderSceneNode(const SceneNode & node, stack<mat4> & matStack,
                         bool isPicking) {
  //cout << "sceneNode: " << node << " (" << node.children.size() << " children)" << endl;
  mat4 jointRotationMat;  
  bool isSelectedJoint = false;
  if (node.m_nodeType == NodeType::JointNode) {
    if (m_selectedJoints.find(node.m_nodeId) != m_selectedJoints.end()) {
      isSelectedJoint = true;
    }
    
    JointRotation jr = getJointRotation(node.m_nodeId);
    jointRotationMat = glm::rotate(jointRotationMat,
                                   (float)degreesToRadians(jr.x),
                                   vec3(1, 0, 0));
    jointRotationMat = glm::rotate(jointRotationMat,
                                   (float)degreesToRadians(jr.y),
                                   vec3(0, 1, 0));
  }
  
  matStack.push(matStack.top() * node.trans * jointRotationMat);
  
  for (const SceneNode * child : node.children) {
    switch(child->m_nodeType) {
      case NodeType::GeometryNode: {
        const GeometryNode * geometryNode =
          static_cast<const GeometryNode *>(child);
        if (isPicking) {
          pickingGeometryNode(*geometryNode, matStack.top());
        }
        else {
          renderGeometryNode(*geometryNode, matStack.top(), isSelectedJoint);
        }
        break;
      }
      /*case NodeType::JointNode: {
        cout << "jointnode: " << *child << "------------" << endl;
        for (auto it = child->children.begin(); 
             it != child->children.end(); it++) {
          cout << **it << endl;     
        }
        cout << "------------------------------------" << endl;
        break;
      }*/
    }

    renderSceneNode(*child, matStack, isPicking);
	}
	matStack.pop();
}

//----------------------------------------------------------------------------------------
void A3::renderGeometryNode(const GeometryNode & node, mat4 modelMat,
                            bool isHighlighted) {  
  //cout << "geometryNode: " << node << endl;  
  updateShaderUniforms(m_shader, node, m_view, modelMat, false, isHighlighted,
                       m_highlight_timer.timer, HIGHLIGHT_STRENGTH);

	// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
	BatchInfo batchInfo = m_batchInfoMap[node.meshId];

	//-- Now render the mesh:
	m_shader.enable();	
	  glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
void A3::pickingGeometryNode(const GeometryNode & node, mat4 modelMat) {
  updateShaderUniforms(m_shader_picking, node, m_view, modelMat, true, false,
                       0, 0);
  
  // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
	BatchInfo batchInfo = m_batchInfoMap[node.meshId];

  vec3 idColor = idToColor(node.m_nodeId);

  m_shader_picking.enable();
    glUniform4f(m_picking_col_uni, idColor.x, idColor.y, idColor.z, 1.0f);
    glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
  m_shader_picking.disable();
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

  vec2 old_coords = m_mouse_coords;
  m_mouse_coords = vec2 (
			(2.0f * xPos) / m_windowWidth - 1.0f,
			1.0f - ( (2.0f * yPos) / m_windowHeight)
	);
	vec2 old_raw_coords = m_raw_mouse_coords;
	m_raw_mouse_coords = vec2(xPos, m_windowHeight - yPos);
	vec2 delta = m_mouse_coords - old_coords;
	
	if (!ImGui::IsMouseHoveringAnyWindow()) {
	  switch((InteractionMode)m_mode) {
	    case POSITION_ORIENTATION: {
	      movePositionOrientation(delta.x, delta.y, old_raw_coords,
	                              m_raw_mouse_coords);
	      break;
      }
	    case JOINTS: moveJoints(delta.x, delta.y); break;
	  }
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (actions == GLFW_PRESS) {
	  switch (button) {
	    case GLFW_MOUSE_BUTTON_LEFT: m_mouseLeftDown = true; break;
	    case GLFW_MOUSE_BUTTON_RIGHT: m_mouseRightDown = true; break;
	    case GLFW_MOUSE_BUTTON_MIDDLE: m_mouseMiddleDown = true; break;
	  }
	  
	  switch (button) { // tell parent event handled
	    case GLFW_MOUSE_BUTTON_LEFT:
	    case GLFW_MOUSE_BUTTON_RIGHT:
	    case GLFW_MOUSE_BUTTON_MIDDLE:
	      eventHandled = true;
	  }
	}
	
	if (actions == GLFW_RELEASE) {
	  switch (button) {
	    case GLFW_MOUSE_BUTTON_LEFT: {
	      m_mouseLeftDown = false;
	      if ((InteractionMode)m_mode == JOINTS) {
	        m_isPicking = true;
        }
	      break;
      }
	    case GLFW_MOUSE_BUTTON_RIGHT: m_mouseRightDown = false; break;
	    case GLFW_MOUSE_BUTTON_MIDDLE: {
	      m_mouseMiddleDown = false;
	      pushActiveTransformation();
	      break;
      }
	  }
	  
	  switch (button) { // tell parent event handled
	    case GLFW_MOUSE_BUTTON_LEFT:
	    case GLFW_MOUSE_BUTTON_RIGHT:
	    case GLFW_MOUSE_BUTTON_MIDDLE:
	      eventHandled = true;
	  }
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		switch(key) {
		  case GLFW_KEY_M: {
		    show_gui = !show_gui;
		    break;
		  }
		
		  // Application Menu Shortcuts
		  case 'Q': quit(); break;
		  case 'A': resetAll(); break;
		  case 'N': resetJoints(); break;
		  case 'O': resetOrientation(); break;
		  case 'I': resetPosition(); break;
		
		  // Edit Menu Shortcuts
		  case 'U': undo(); break;
		  case 'R': redo(); break;
		
		  // Options Menu Shortcuts
		  case 'C': m_circle = ! m_circle; break;
		  case 'Z': m_zbuffer = ! m_zbuffer; break;
		  case 'B': m_backface_culling = ! m_backface_culling; break;
		  case 'F': m_frontface_culling = ! m_frontface_culling; break;
		
		  // Radio Button Shortcuts
		  case 'P': m_mode = (int)POSITION_ORIENTATION; break;
		  case 'J': m_mode = (int)JOINTS; break;
		}
		
		// tell parent that key event has been handled
		switch(key) {
		  case GLFW_KEY_M:
		  case 'Q':
		  case 'A':
		  case 'N':
		  case 'O':
		  case 'I':
		  case 'U':
		  case 'R':
		  case 'C':
		  case 'Z':
		  case 'B':
		  case 'F':
		  case 'P':
		  case 'J':
		    eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
void A3::resetAll() {
  cout << "resetAll" << endl;
  resetJoints();
  resetOrientation();
  resetPosition();
}

//----------------------------------------------------------------------------------------
void A3::resetJoints() {
  cout << "resetJoints" << endl;
  m_activeTransformation = m_transformStack.at(0);
  m_transformStack.clear();
  m_transformStackMax = 0;
  m_transformStackTop = 0;
  pushActiveTransformation();
}

//----------------------------------------------------------------------------------------
void A3::resetOrientation() {
  cout << "resetOrientation" << endl;
  getRotationNode()->trans = mat4();
}

//----------------------------------------------------------------------------------------
void A3::resetPosition() {
  cout << "resetPosition" << endl;
  getTranslationNode()->trans = mat4();
}

//----------------------------------------------------------------------------------------
void A3::undo() {
  cout << "undo" << endl;
  bool wasPopped = popTransformation();
  resetActiveTransformation();
  
  bool isCurrentlyRotating = (m_mouseMiddleDown || m_mouseRightDown) &&
                             m_mode == (InteractionMode)JOINTS;
  
  if (! wasPopped && ! isCurrentlyRotating) {
    resetTimer(m_undoWarnTimer, 1.0f / UNDO_REDO_WARNING_DURATION);
  }
}

//----------------------------------------------------------------------------------------
void A3::redo() {
  cout << "redo" << endl;
  bool wasUnpopped = unpopTransformation();
  resetActiveTransformation();
  
  bool isCurrentlyRotating = (m_mouseMiddleDown || m_mouseRightDown) &&
                             m_mode == (InteractionMode)JOINTS;
  
  if (! wasUnpopped && ! isCurrentlyRotating) {
    resetTimer(m_redoWarnTimer, 1.0f / UNDO_REDO_WARNING_DURATION);
  }
}

//----------------------------------------------------------------------------------------
void A3::movePositionOrientation(float dx, float dy, vec2 old_coords,
                                 vec2 new_coords) {
  if (m_mouseLeftDown) {
    translatePuppetXY(dx, dy);
  }
  
  if (m_mouseRightDown) {
    rotatePuppet(old_coords, new_coords);
  }
  
  if (m_mouseMiddleDown) {
    translatePuppetZ(-dy); // negated
  }
}

//----------------------------------------------------------------------------------------
void A3::moveJoints(float dx, float dy) {
  if (m_mouseMiddleDown) {
    rotateJoints(JOINT_ROTATION_SPEED * dx, JOINT_ROTATION_SPEED * dy);
  }
  
  if (m_mouseRightDown) {
    rotateHead(JOINT_ROTATION_SPEED * dx);
  }
}

//----------------------------------------------------------------------------------------
void A3::toggleJointSelection(int jointId) {
  if (m_selectedJoints.find(jointId) == m_selectedJoints.end()) {
    cout << "Select joint " << jointId << endl;
    m_selectedJoints.insert(jointId);
  }
  else {
    cout << "De-select joint " << jointId << endl;
    m_selectedJoints.erase(jointId);    
  }
}

//----------------------------------------------------------------------------------------
void A3::translatePuppetXY(float dx, float dy) {
  getTranslationNode()->translate(vec3(dx, dy, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::translatePuppetZ(float delta) {
  cout << "translate puppet z: " << delta << endl;
  getTranslationNode()->translate(vec3(0.0f, 0.0f, delta));
}

//----------------------------------------------------------------------------------------
void A3::rotatePuppet(vec2 from, vec2 to) {
  float radius = glm::min(m_windowWidth, m_windowHeight) / 4.0f;
  vec2 origin = vec2(m_windowWidth / 2.0f, m_windowHeight / 2.0f);
  from = from - origin;
  to = to - origin;
  if (length(from) > radius) {
    from = (from / length(from)) * radius;
  }
  
  if (length(to) > radius) {
    to = (to / length(to)) * radius;
  }
  
  vec3 from3d = vec3(from.x,
                     from.y,
                     glm::max(0.0,
                              sqrt(glm::max(0.0, glm::pow(radius, 2) -
                                                 glm::pow(length(from), 2)))));
  vec3 to3d = vec3(to.x,
                   to.y,
                   glm::max(0.0,
                            sqrt(glm::max(0.0, glm::pow(radius, 2) -
                                               glm::pow(length(to), 2)))));

  if (from3d != to3d) {                         
    vec3 axis = normalize(cross(from3d, to3d));
    float angle = acos(dot(from3d, to3d) / (length(from3d) * length(to3d)));
    
    if (! isnan(angle) && ! isnan(axis.x) && ! isnan(axis.y) &&
        ! isnan(axis.z)) {
      cout << "axis " << axis << endl;
      cout << "angle " << angle << endl;
      cout << "length from " << length(from3d) << endl;
      cout << "length to " << length(to3d) << endl;
      
      mat4 rotation;
      rotation = glm::rotate(rotation, angle, axis);
      getRotationNode()->trans = rotation * getRotationNode()->trans;
      
      cout << getRotationNode()->trans << endl;
    }
  }
}

//----------------------------------------------------------------------------------------
// Precond & Postcond: m_activeTransformation is valid (i.e. contains an entry
// for every joint)
void A3::rotateJoints(float dx, float dy) {
  cout << "rotate joints: ================================" << endl;
  for (auto it = m_selectedJoints.begin(); it != m_selectedJoints.end(); it++) {
    JointRotation jr = getJointRotation(*it);
    
    cout << dy << endl;
    //cout << "from (" << jr.x << "," << jr.y << ")";
    
    jr.x += dx;
    jr.y += dy;
    
    if (jr.x < jr.xRange.min) {
      jr.x = jr.xRange.min;
    }
    else if (jr.x > jr.xRange.max) {
      jr.x = jr.xRange.max;
    }
    
    if (jr.y < jr.yRange.min) {
      jr.y = jr.yRange.min;
    }
    else if (jr.y > jr.yRange.max) {
      jr.y = jr.yRange.max;
    }

    //cout << " to (" << jr.x << "," << jr.y << ")" << endl;

    m_activeTransformation[*it] = jr;
  }
}

//----------------------------------------------------------------------------------------
void A3::rotateHead(float delta) {
  cout << delta << endl;
  JointRotation jr = getJointRotation(m_headJointId);
  jr.y += delta;
  
  if (jr.y < jr.yRange.min) {
    jr.y = jr.yRange.min;
  }
  else if (jr.y > jr.yRange.max) {
    jr.y = jr.yRange.max;
  }
  
  m_activeTransformation[m_headJointId] = jr;
}

//----------------------------------------------------------------------------------------
SceneNode * A3::getTranslationNode() {
  return & * m_rootNode;
}

//----------------------------------------------------------------------------------------
SceneNode * A3::getRotationNode() {
  return getTranslationNode()->children.front()->children.front();
}

//----------------------------------------------------------------------------------------
glm::vec3 A3::idToColor(int id){
  float r = ((id & 0x000000FF) >> 0) / 255.0f;
  float g = ((id & 0x0000FF00) >> 8) / 255.0f;
  float b = ((id & 0x00FF0000) >> 16) / 255.0f;
  return vec3(r, g, b);
}

//----------------------------------------------------------------------------------------
int A3::colorToId(unsigned char color[4]) {
  return color[0] +
         color[1] * 256 +
         color[2] * 256 * 256; 
}

//----------------------------------------------------------------------------------------
void A3::resetTimer(OscillatingTimer & timer, float tick) {
  timer.timer = 0.0f;
  timer.isIncreasing = true;
  timer.tick = tick;
}

//----------------------------------------------------------------------------------------
void A3::tickTimer(OscillatingTimer & timer) {
  if (timer.isIncreasing) {
    timer.timer += timer.tick;
  }
  else {
    timer.timer -= timer.tick;
  }
  
  if (timer.timer <= 0.0f) {
    timer.timer = 0.0f;
    timer.isIncreasing = true;
  }
  else if (timer.timer >= 1.0f) {
    timer.timer = 1.0f;
    timer.isIncreasing = false;
  }
}

//----------------------------------------------------------------------------------------
void A3::resetTimer(CountdownTimer & timer, float tick) {
  timer.timer = 1.0f;
  timer.tick = tick;
}

//----------------------------------------------------------------------------------------
void A3::tickTimer(CountdownTimer & timer) {
  timer.timer -= timer.tick;
  if (timer.timer < 0.0f) {
    timer.timer = 0.0f;
  }
}

//----------------------------------------------------------------------------------------
void A3::pushActiveTransformation() {
  auto stackTopIter = m_transformStack.begin() + m_transformStackTop;
  m_transformStack.insert(stackTopIter, m_activeTransformation);
  m_transformStackTop++;
  m_transformStackMax = m_transformStackTop;
}

//----------------------------------------------------------------------------------------
void A3::resetActiveTransformation() {
  auto stackTopIter = m_transformStack.begin() + m_transformStackTop;
  m_activeTransformation = m_transformStack.at(m_transformStackTop - 1);
}

//----------------------------------------------------------------------------------------
// Return whether there was anything to pop
bool A3::popTransformation() {
  m_transformStackTop--;
  if (m_transformStackTop < 1) {
    m_transformStackTop = 1;
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
// Return whether there was anything to unpop
bool A3::unpopTransformation() {
  m_transformStackTop++;
  if (m_transformStackTop > m_transformStackMax) {
    m_transformStackTop = m_transformStackMax;
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
JointRotation A3::getJointRotation(int jointId) {
  return m_activeTransformation[jointId];
}

//----------------------------------------------------------------------------------------
void A3::quit() {
  glfwSetWindowShouldClose(m_window, GL_TRUE);
}
