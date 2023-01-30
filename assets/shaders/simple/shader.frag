#version 430

out vec4 color;
layout (location = 2) in float dist;
void main() {
     color = vec4(3/(dist*dist + dist), 3/(dist*dist),  3/(dist*dist),  1.0f);
}