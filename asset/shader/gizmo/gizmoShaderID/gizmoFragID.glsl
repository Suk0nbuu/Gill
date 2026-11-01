#version 330

out uint fragColor;

uniform uint GizmoAxis;
void main() {
    fragColor = GizmoAxis;
}
