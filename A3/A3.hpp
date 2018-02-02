#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <stack>
#include <map>
#include <set>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

struct OscillatingTimer {
  float timer;
  float tick;
  bool isIncreasing;
};

struct CountdownTimer {
  float timer;
  float tick;
};

struct JointRotation {
  double x;
  double y;
  JointNode::JointRange xRange;
  JointNode::JointRange yRange;
};

class A3 : public CS488Window {
public:
  enum InteractionMode { POSITION_ORIENTATION, JOINTS };

  // number of frames for a full period in highlighting
  const float HIGHLIGHT_INTERVAL = 50.0f;
  const float HIGHLIGHT_STRENGTH = 0.6f; // between 0.0f and 1.0f
  const double JOINT_ROTATION_SPEED = 20.0f;
  const float UNDO_REDO_WARNING_DURATION = 200.0f; // in # of frames
  const std::string UNDO_WARNING = "Nothing to Undo! ";
  const std::string REDO_WARNING = "Nothing to Redo! ";
  const std::string HEAD_JOINT_NAME = "neckbase";

	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();
	void initGeometryJointMap();
	void initGeometryJointMapHelper(const SceneNode & node);
	void initTransformStack();
	void initTransformStackHelper(const SceneNode & node);

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void picking(const SceneNode & node);
	void renderSceneGraph(const SceneNode & node, bool isPicking = false);
	void renderSceneNode(const SceneNode & node,
	                     std::stack<glm::mat4> & matStack, bool isPicking);
	void renderGeometryNode(const GeometryNode & node, glm::mat4 mat,
	                        bool isHighlighted);
	void pickingGeometryNode(const GeometryNode & node, glm::mat4 modelMat);
	void renderArcCircle();

  //-- ImGui Menus
  void showApplicationMenu();
  void showEditMenu();
  void showOptionsMenu();

  //-- Application Menu
  void resetAll();
  void resetJoints();
  void resetOrientation();
  void resetPosition();
  void quit();
  
  // Edit Menu
  void undo();
  void redo();
  CountdownTimer m_undoWarnTimer;
	CountdownTimer m_redoWarnTimer;
  
  // Interaction Modes
  void movePositionOrientation(float dx, float dy, glm::vec2 old_coords,
                               glm::vec2 new_coords);
  void moveJoints(float dx, float dy);
  // Position/Orientation Mode Impl.
  void translatePuppetXY(float dx, float dy);
  void translatePuppetZ(float delta);
  void rotatePuppet(glm::vec2 from, glm::vec2 to);
  // Joints Mode Impl.
  void rotateJoints(float dx, float dy);
  void rotateHead(float delta);
  
  //-- Access special nodes in puppet
  SceneNode * getTranslationNode();
  SceneNode * getRotationNode();

  //-- Picking utility methods
  glm::vec3 idToColor(int id);
  int colorToId(unsigned char color[4]);
   // given a geometry node id, return ptr to its joint parent, or NULL
  std::shared_ptr<JointNode> idToJoint(int geoNodeId);
  void toggleJointSelection(int jointId);
  const SceneNode * findNode(int nodeId);
  
  // Timer-related
  void resetTimer(OscillatingTimer & timer, float tick);
  void tickTimer(OscillatingTimer & timer);
  void resetTimer(CountdownTimer & timer, float tick);
  void tickTimer(CountdownTimer & timer);

  // Transform Stack
  void pushActiveTransformation();
  void resetActiveTransformation();
  bool popTransformation();
  bool unpopTransformation();
  void resetStack();
  JointRotation getJointRotation(int jointId);

  // Members =================================================================

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;
	
	//-- GL resources for picking
	GLuint m_picking_col_uni; // uniform location for picking color
	GLint m_pickingPositionAttribLocation;
	ShaderProgram m_shader_picking;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;
	
	int m_mode; // interaction mode
	
	// Option Menu
	bool m_circle;
	bool m_zbuffer;
	bool m_backface_culling;
	bool m_frontface_culling;
	
	// Mouse Button Events
	bool m_mouseLeftDown;	
	bool m_mouseRightDown;
	bool m_mouseMiddleDown;

  // picking related
	bool m_isPicking; // is picking mode for this current frame
	// map of geometryNode IDs to parent JointNode IDs
	std::map<int, int> m_geometryJointMap;
	// set of JointNode IDs which are selected
	std::set<int> m_selectedJoints;
	// vector-stack of joint rotations
	std::vector<std::map<int, JointRotation> > m_transformStack;
	// index to top of transform stack
	int m_transformStackTop;
	// index to maximum transformation in stack
	int m_transformStackMax;
	// active set of joint rotations (while mouse is held down)
	std::map<int, JointRotation> m_activeTransformation;
	int m_headJointId;
	
	
  glm::vec2 m_mouse_coords; // Mouse coords	
  glm::vec2 m_raw_mouse_coords; // Non-normalized Mouse coords
  
  OscillatingTimer m_highlight_timer;
};
