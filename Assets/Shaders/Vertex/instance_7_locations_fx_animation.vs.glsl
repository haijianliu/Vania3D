
#version 330 core
layout (location = 0) in vec3 positionVertex;
layout (location = 1) in vec2 uvVertex;
layout (location = 2) in vec3 normalVertex;
layout (location = 3) in vec4 instanceMatrix0;
layout (location = 4) in vec4 instanceMatrix1;
layout (location = 5) in vec4 instanceMatrix2;
layout (location = 6) in vec4 instanceMatrix3;
layout (location = 7) in float animationTime;

out vec3 position;
out vec2 uv;
out vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 animation; // divideX, divideY, sampleTime(frame per second)

void main() {
	mat4 model;
	model[0] = instanceMatrix0;
	model[1] = instanceMatrix1;
	model[2] = instanceMatrix2;
	model[3] = instanceMatrix3;

	position = vec3(model * vec4(positionVertex, 1.0));
	// uv = uvVertex;
	uv.x = uvVertex.x / animation.x;
	uv.y = uvVertex.y / animation.y;
	normal = vec3(model * vec4(normalVertex, 0.0));

	gl_Position =  projection * view * model * vec4(positionVertex, 1.0);
}
