uniform sampler2D      displacementMap;

void main()
{
    float scale = 1.0;
    vec4 newVertexPos;
    vec4 dv;
    dv = texture2D( displacementMap, gl_MultiTexCoord0.xy );
    newVertexPos = vec4(scale*dv.x, scale*dv.y, scale*dv.z, 1);
    gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
    
    // make texture coordinate available to fragment shader
    gl_TexCoord[0] = gl_MultiTexCoord0;
}