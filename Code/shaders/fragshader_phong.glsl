#version 330 core

// Define constants
#define M_PI 3.141593

// The input from the vertex shader.
in vec3 vertNormal;
in vec3 vertPosition;
in vec3 relativeLightPosition;
in vec2 texCoords;

// Lighting model constants.
uniform vec4 material;
uniform vec3 lightColour;

// Texture sampler
uniform sampler2D texture1Sampler;

// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 vertColour;

void main()
{
  // Ambient colour does not depend on any vectors.
  vec3 texColour = texture2D(texture1Sampler, texCoords).xyz;
  vec3 colour    = material.x * texColour;

  // Calculate light direction vectors in the phong model.
  vec3 lightDirection   = normalize(relativeLightPosition - vertPosition);
  vec3 normal           = normalize(vertNormal);

  // Diffuse colour.
  float diffuseIntesity = max(dot(normal, lightDirection), 0);
  colour += texColour * material.y * diffuseIntesity;

  // Specular colour.
  vec3 viewDirection     = normalize(-vertPosition); // The camera is always at (0, 0, 0).
  vec3 reflectDirection  = reflect(-lightDirection, normal);
  float specularIntesity = max(dot(reflectDirection, viewDirection), 0);
  colour += texColour * lightColour * material.z * pow(specularIntesity, material.w);

  vertColour = vec4(colour, 1);
}
