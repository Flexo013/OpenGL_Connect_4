#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout (location = 0) in vec3 vertCoordinates_in;
layout (location = 1) in vec3 vertNormals_in;
layout (location = 2) in vec2 texCoords_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelViewTransform;
uniform mat4 projectionTransform;
uniform vec3 lightPosition;
uniform mat3 normalTransform;

// Specify the output of the vertex stage
out vec3 vertNormal;
out vec3 vertPosition;
out vec3 relativeLightPosition;
out vec2 texCoords;

void main()
{
    gl_Position  = projectionTransform * modelViewTransform * vec4(vertCoordinates_in, 1.0);

    // Pass the required information to the fragment stage.
    relativeLightPosition = vec3(modelViewTransform * vec4(lightPosition, 1));
    vertPosition = vec3(modelViewTransform * vec4(vertCoordinates_in, 1));
    vertNormal   = normalize(normalTransform * vertNormals_in);
    texCoords    = texCoords_in;
}
