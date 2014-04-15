uniform sampler2D      textureMap;

void main(void)
{
    gl_FragColor.rgb = texture2D( textureMap, gl_TexCoord[0].st ).rgb;
    gl_FragColor.a = 0.001;
}
