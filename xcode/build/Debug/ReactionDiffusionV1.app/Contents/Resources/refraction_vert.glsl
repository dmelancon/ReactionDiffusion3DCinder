#version 120
uniform float scale;
uniform int size;
uniform sampler2D displacementMap;
varying vec3 color;
void main(void)
{
vec4 newVertexPos;
vec4 dv;
float df;

gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

dv = texture2D( displacementMap, gl_MultiTexCoord0.xy/size );

df = .1*dv.x + 0.1*dv.y + 0.1*dv.z;

newVertexPos = vec4(gl_Normal * dv.r * scale, 0.0) + gl_Vertex;
    color = mix(vec3(0.),vec3(.8), dv.b );

gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
}
