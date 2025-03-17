#version 450

vec2 vertices[3] = vec2[](
    vec2( 0.0, -0.5),
    vec2(-0.5,  0.5),
    vec2( 0.5,  0.5)
);

vec3 vertColors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout (location = 0) out vec3 vertColor;

void main(){
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
    vertColor = vertColors[gl_VertexIndex];
}