#version 120


uniform sampler2D displacementMap;
varying vec3 color;
void main(void)
{
vec4 newVertexPos;
vec4 dv;
float df;

gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

dv = texture2D( displacementMap, gl_MultiTexCoord0.xy );

df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z;

newVertexPos = vec4(gl_Normal * df * 100.0, 0.0) + gl_Vertex;
    color = mix(vec3(1.),vec3(0.), dv.r );

gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
}
