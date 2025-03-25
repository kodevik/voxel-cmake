#pragma once

const char *blockVert = R"(
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 offs;
layout (location = 3) in int txPos;
layout (location = 4) in int side;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 rotations[6];

out vec2 fTexCoord;

void main()
{
    vec4 newPos = rotations[side] * vec4(pos, 1.0) + vec4(offs+0.5, 0.0);
    gl_Position = projection * view * newPos;
    float txOffs = side == 2 ? 0.0 : side == 1 ? 0.25 : 0.5;
    fTexCoord = vec2(txOffs + texCoord.x * 0.25, float(txPos) * 0.25 + texCoord.y*0.25);
}
)";

const char *blockFrag = R"(
#version 330 core

in vec2 fTexCoord;
uniform sampler2D txtr;

void main() {
    gl_FragColor = texture(txtr, fTexCoord);
}
)";