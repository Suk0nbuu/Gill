#version 330

out uint fragColor;

uniform uint ObjectID;
void main() {
    fragColor = ObjectID;
}
