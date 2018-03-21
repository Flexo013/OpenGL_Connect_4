#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the inputs to the fragment shader
// These must have the same type and name!
in float ambient, diffuse, specular;
in vec2 texCoords;

// Specify the Uniforms of the fragment shaders
uniform sampler2D textureSampler;
uniform vec3 lightColour;

// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColour;

void main()
{
  vec3 texColor = texture2D(textureSampler, texCoords).xyz;

  // Combine the received components into one colour.
  fColour = vec4(ambient * texColor + (diffuse + specular) * lightColour * texColor, 1);
}
