attribute vec2 position;
attribute vec2 texcoord;

uniform mat4 projection;
uniform mat4 model;

varying vec2 texcoord_out;

void main()
{
    gl_Position = projection * model * vec4(position, 0.0, 1.0);
    texcoord_out = texcoord;
}
