#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "read_file.h"
#include "math_functions.h"
#include "vertex_array.h"
#include "gl_wrappers.h"


float scroll_value = 1.0F;
void scroll_function(GLFWwindow *window, double x, double y) {
    scroll_value += (2.0F*(float)y)/100.0F;
}


struct mouse_positions {
    int active;
    float prev[2];
    float curr[2];
    float change[3];
};


void on_mouse_press_rotate(
    struct mouse_positions *mouse_positions,
    float *camera_vector, float *q_axis_array) {
    float rotation_axis[3] = {0.0};
    float q_rotation_axis[4] = {1.0F, 0.0F, 0.0F, 0.0F};
    if (
        mouse_positions->change[0] != 0.0F || 
        mouse_positions->change[1] != 0.0F) 
    {
        cross_product(rotation_axis, camera_vector, mouse_positions->change);
        float angle = 15.0*sqrt(
            mouse_positions->change[0]*mouse_positions->change[0] + 
            mouse_positions->change[1]*mouse_positions->change[1]
            );
        quaternion_rotation_axis(q_rotation_axis, angle, rotation_axis);
        quaternion_multiply_inplace(q_axis_array, q_rotation_axis);
    }
}


void movement(float *v3d, GLFWwindow *window) {
    if (glfwGetKey(window, 'W') == GLFW_PRESS) {
        v3d[1] += 1.0;
    }
    if (glfwGetKey(window, 'A') == GLFW_PRESS) {
        v3d[0] += -1.0;
    }
    if (glfwGetKey(window, 'S') == GLFW_PRESS) {
        v3d[1] += -1.0;
    }
    if (glfwGetKey(window, 'D') == GLFW_PRESS) {
        v3d[0] += 1.0;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        v3d[2] += 1.0;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        v3d[2] -= 1.0;
    }
    // printf("%f, %f\n", v3d[0], v3d[1]);
}


void movement_clear(float *v3d) {
    v3d[0] = 0.0;
    v3d[1] = 0.0;
    v3d[2] = 0.0;
}


float y_orientation[3] = {0.0F, 1.0F, 0.0F};
void on_mouse_press_rotate_preserve_orientation (
    struct mouse_positions *mouse_positions,
    float *camera_vector,
    float *q_axis_array) {
    float rotation_axis[3] = {0.0};
    float q_rotation_axis[4] = {1.0F, 0.0F, 0.0F, 0.0F};
    if (mouse_positions->change[1] != 0.0F) {
        float y_change[3] = {0.0F};
        y_change[1] = mouse_positions->change[1];
        cross_product(rotation_axis, camera_vector, y_change);
        float angle = 15.0*mouse_positions->change[1];
        angle = sqrt(angle*angle);
        quaternion_rotation_axis(q_rotation_axis, angle, rotation_axis);
        quaternion_multiply_inplace(q_axis_array, q_rotation_axis);
    }
    quaternion_rotate3(y_orientation, q_rotation_axis, y_orientation);
    if (mouse_positions->change[0] != 0.0F) {
        float angle = 15.0*mouse_positions->change[0];
        quaternion_rotation_axis(q_rotation_axis, angle, y_orientation);
        quaternion_multiply_inplace(q_axis_array, q_rotation_axis);
    }
}



void on_mouse_press(
    struct mouse_positions *mouse_positions, double *mouse,
    float *camera_vector, float *q_axis_array, int w, int h
    ) {
    mouse_positions->curr[0] = mouse[0]/w - 0.5;
    mouse_positions->curr[1] = 0.5 - mouse[1]/h;
    if (!mouse_positions->active) {
        mouse_positions->active = 1;
        mouse_positions->prev[0] = mouse[0]/w - 0.5;
        mouse_positions->prev[1] = 0.5 - mouse[1]/h;
    } else {
        mouse_positions->change[0] = mouse_positions->curr[0]; 
        mouse_positions->change[0] -= mouse_positions->prev[0];
        mouse_positions->change[1] = mouse_positions->curr[1];
        mouse_positions->change[1] -= mouse_positions->prev[1];
        mouse_positions->change[2] = 0.0; 
        mouse_positions->prev[0] = mouse_positions->curr[0];
        mouse_positions->prev[1] = mouse_positions->curr[1]; 
    }
    if (mouse_positions->active) {
        // on_mouse_press_rotate(mouse_positions, camera_vector, q_axis_array);
        on_mouse_press_rotate_preserve_orientation(
            mouse_positions, camera_vector, q_axis_array);
    }
}



int main() {

    // Get contents of shader files.
    char *file_contents = get_file_contents("./shaders/vertices.vert");
    if (file_contents == NULL) {
        fprintf(stderr, "Unable to open vertex_shader.vert");
        exit(1);
    }
    const char *v_source = file_contents;
    char *file_contents2 = get_file_contents("./shaders/fragments.frag");
    if (file_contents2 == NULL) {
        fprintf(stderr, "Unable to open fragment_shader.frag");
        exit(1);
    }
    const char *f_source = file_contents2;

    // Initialize GLFW
    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(800, 800, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error.\n");
        exit(1);
    }

    int image_w = 0;
    int image_h = 0;
    unsigned char *image = get_bitmap_contents(
        "./textures/world.bmp", &image_w, &image_h);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    struct vertices v;
    int len = 150;
    float buff[len*len*12*6];
    vertices_init(&v, len*len*6, buff, NULL);
    make_sphere(&v, 0.5, len, len);


    // make_cylinder(&v, 0.5, sqrtf(2.0*0.0625), numberOfPointsPerEdge);
    float *vertices = vertices_get_array(&v);
    glGenBuffers(1, &vbo);  
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 
    vertices_array_get_size(&v),
    vertices, GL_STATIC_DRAW);

    /*GLuint vao2;
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    GLuint vbo2;
    glGenBuffers(1, &vbo2);  
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    vertices = vertices_get_array(&v2);
    glBufferData(GL_ARRAY_BUFFER, 
    vertices_array_get_size(&v2)
    + vertices_array_get_size(&v3)
    + vertices_array_get_size(&v4)
    + vertices_array_get_size(&v5)
    + vertices_array_get_size(&v6), 
    vertices, GL_STATIC_DRAW);*/

    GLuint vs_ref = make_vertex_shader(v_source);
    GLuint fs_ref = make_fragment_shader(f_source);

    GLuint program = make_program(vs_ref, fs_ref);

    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    unsigned char backup_texture[4] = {
        255, 25, 25, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                 1, 1, 0, GL_RGB, 
                 GL_UNSIGNED_BYTE, backup_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                 image_w, image_h, 0, GL_RGB, 
                 GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    
    GLint pos_attrib = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, v.vertex_size*sizeof(float), 0);

    GLint norm_attrib = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(norm_attrib);
    glVertexAttribPointer(norm_attrib, 3, GL_FLOAT, GL_FALSE, v.vertex_size*sizeof(float),
                          (void *)(v.normal_offset*sizeof(float)));

    GLint tex_attrib = glGetAttribLocation(program, "texture");
    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, v.vertex_size*sizeof(float),
                          (void *)(v.uv_offset*sizeof(float)));

    GLint col_attrib = glGetAttribLocation(program, "inColour");
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 
                          v.vertex_size*sizeof(float), 
                          (void *)(v.color_offset*sizeof(float)));

    GLint rotation = glGetUniformLocation(program, "rotation");
    GLint q_axis = glGetUniformLocation(program, "qAxis");
    GLint movement_3d = glGetUniformLocation(program, "movement3D");
    GLint scale = glGetUniformLocation(program, "scale");
    GLint tex = glGetUniformLocation(program, "tex");
    glUniform1i(tex, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glUniform1f(scale, 1.0f);
    float rotation_array[16] = {0.0};
    float q_axis_array[4] = {1.0, 0.0, 0.0, 0.0};
    rotation_transform4(rotation_array, 0.2, 0.0, 0.0);
    double phi;
    double mouse[2] = {0};
    struct mouse_positions mouse_positions = {0, {0.0}, {0.0}, {0.0}};
    mouse_positions.active = 0;
    int w, h;
    float camera_vector[3] = {0.0, 0.0, 1.0};
    float v3d[3] = {0.0, 0.0, 0.0};
    glfwSetScrollCallback(window, scroll_function);
    for (int i = 0; !glfwWindowShouldClose(window); i++) {
        phi = i/60.0;
        glClearColor(0.0, 0.0, 0.0, 1.0);
        rotation_transform4(rotation_array, phi, phi/2, 0.0);
        glUniform1f(scale, scroll_value);
        glUniform4f(
            q_axis, 
            q_axis_array[0], q_axis_array[1], 
            q_axis_array[2], q_axis_array[3]
            );
        glUniformMatrix4fv(rotation, 1, 0, rotation_array);
        movement(v3d, window);
        // v3d[0] = 0.0;
        // v3d[1] = 0.0;
        glUniform3f(movement_3d, v3d[0], v3d[1], v3d[2]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, v.n*v.vertex_size);
        // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwGetCursorPos(window, &mouse[0], &mouse[1]);
        glfwGetFramebufferSize(window, &w, &h);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            on_mouse_press(
                &mouse_positions, mouse, camera_vector, q_axis_array, w, h);
        } else {
            mouse_positions.active = 0;
        }
    }

    // TODO: clean up memory and properly close things
    free((char *)v_source);
    free((char *)f_source);
    free(image);
    glDeleteShader(vs_ref);
    glDeleteShader(fs_ref);
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glfwTerminate();

    return 0;
}

