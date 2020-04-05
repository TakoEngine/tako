attribute vec2 position;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(position, 0.0, 1.0);
}
