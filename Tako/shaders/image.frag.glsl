uniform sampler2D texture;
uniform vec4 color;

varying vec2 texcoord_out;

void main()
{
    gl_FragColor = texture2D(texture, texcoord_out) * color;
}
