#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;

// Values that stay constant for the whole mesh.
uniform mat4 ModelView;
uniform mat4 Perspective;

void main(){
  // Output position of the vertex, in clip space : MVP * position
  gl_Position = Perspective * ModelView * vec4(position, 1);
}
