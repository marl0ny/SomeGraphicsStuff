# version 330 core

layout (TRIANGLES) in;
layout (TRIANGLE_STRIP, max_vertices=3) out;

in VS_GS_VERTEX {
    vec3 outColour;
} vertex_in[];

out GS_FS_VERTEX {
    vec3 normal;
    vec3 colour2;
} vertex_out;


vec3 crossProduct(vec3 a, vec3 b) {
    return vec3(
        a.y*b.z - a.z*b.y, 
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
        );
}


void main() {
    vec4 sideLength1 = gl_in[1].gl_Position - gl_in[0].gl_Position;
    vec4 sideLength2 = gl_in[2].gl_Position - gl_in[1].gl_Position;
    // gl_Position = gl_in[0].gl_Position;
    // EmitVertex();
    for (int n = 0; n < gl_in.length(); n++) {
        vertex_out.normal = crossProduct(sideLength1.xyz, sideLength2.xyz);
        vec3 outColour = vertex_in[n].outColour;
        vertex_out.colour2 = outColour;
        gl_Position = gl_in[n].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}