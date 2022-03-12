#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "simple_cube_helper_functions.h"


/*
* Compile the shader, given the reference to the shader
* and the shader source code.
*/
void compile_shader(GLuint shader_ref, const char *shader_source) {
    char buf[512];
    glShaderSource(shader_ref, 1, &shader_source, NULL);
    glCompileShader(shader_ref);
    GLint status;
    glGetShaderiv(shader_ref, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(shader_ref, 512, NULL, buf);
    if (status == GL_TRUE) {
        if (buf[0] != '\0') {
            fprintf(stdout, "%s", buf);
        }
    } else {
        fprintf(stderr, "%s\n%s", "Shader compilation failed:", buf);
    }
}


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


int main() {

    // Get contents of shader files.
    char *file_contents = get_file_contents("./vertex.vert");
    if (file_contents == NULL) {
        fprintf(stderr, "Unable to open vertex.vert");
        exit(1);
    }
    const char *v_source = file_contents;
    char *file_contents2 = get_file_contents("./fragment.frag");
    if (file_contents2 == NULL) {
        fprintf(stderr, "Unable to open fragment.frag");
        exit(1);
    }
    const char *f_source = file_contents2;
    char *file_contents3 = get_file_contents("./geometry.geom");
    if (file_contents3 == NULL) {
        fprintf(stderr, "Unable to open geometry.geom");
        exit(1);
    }
    const char *g_source = file_contents3;

    // Initialize GLFW
    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version. This is NECESSARY
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version. This is NECESSARY 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ???
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // ???
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(600, 600, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    /*glewExperimental = GL_TRUE; // This is NECESSARY
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error.\n");
        exit(1);
    }*/
    
    // Vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vertex buffer object
    // When assigning the number of values to each vertices,
    // the second parameter of glVertexAttribPointer
    // needs to be changed.
    GLuint vbo;
    float vertices[] = {
          0.25,  0.25, -0.25, 0.0, 1.0, 0.0,
          0.25, -0.25, -0.25,  0.0, 1.0, 0.0, 
          -0.25,  0.25, -0.25,  0.0, 1.0, 0.0,
         -0.25, -0.25,  -0.25,  0.0, 1.0, 0.0,  
          0.25, -0.25, 0.25,  0.0, 1.0, 0.0,
          0.25, 0.25, 0.25,  0.0, 1.0, 0.0,
          -0.25, -0.25, 0.25, 0.0, 1.0, 0.0,
          -0.25, 0.25, 0.25, 0.0, 1.0, 0.0
        };
    glGenBuffers(1, &vbo);  
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // This specifies the vertices to use when drawing the shape.
    // Note that the second parameter in glDrawElements must be changed in order
    // to accommodate for a resizing of the elements.
    GLuint elements[] = {
        0, 1, 2, 
        2, 3, 1, 
        1, 4, 0, 
        0, 5, 4,
        4, 5, 6,
        6, 5, 7,
        7, 6, 3,
        3, 2, 7,
        0, 2, 7,
        7, 5, 0,
        1, 3, 4,
        4, 6, 3
    };
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Create and compile the vertex shader.
    GLuint vs_ref = glCreateShader(GL_VERTEX_SHADER);
    if (vs_ref == 0) {
        fprintf(stderr, "unable to "
                "create vertex shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    compile_shader(vs_ref, v_source);

    // Create and compile the fragment shader.
    GLuint fs_ref = glCreateShader(GL_FRAGMENT_SHADER);
    if (fs_ref == 0) {
        fprintf(stderr, "Error: unable to "
                "create fragment shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    compile_shader(fs_ref, f_source);


    // Create and compile the geometry shader.
    GLuint gs_ref = glCreateShader(GL_GEOMETRY_SHADER);
    if (gs_ref == 0) {
        fprintf(stderr, "Error: unable to "
                "create geometry shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    compile_shader(gs_ref, g_source);

    // Create the shader program and attach the shaders.
    GLuint program = glCreateProgram();
    glAttachShader(program, vs_ref);
    glAttachShader(program, fs_ref);
    glAttachShader(program, gs_ref);
    glBindFragDataLocation(program, 0, "colour"); 
    glLinkProgram(program);
    glUseProgram(program);

    // The next following lines of code dictate how the
    // vertex array is read.
    GLint pos_attrib = glGetAttribLocation(program, "position");
    // first parameter reference to the input
    // second parameter number of arguments
    // third parameter the type of each component
    // fourth parameter whether input should be normalized
    // fifth parameter number of bytes between each position in the array
    // sixth parameter the offset of the attribute
    glEnableVertexAttribArray(pos_attrib);
    // Specifies how the data from the input is retrieved from the array.
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

    GLint col_attrib = glGetAttribLocation(program, "inColour");
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 
                          6*sizeof(float), (void *)(3*sizeof(float)));
    GLint rotation = glGetUniformLocation(program, "rotation");
    GLint q_axis = glGetUniformLocation(program, "qAxis");
    GLint scale = glGetUniformLocation(program, "scale");

    glEnable(GL_DEPTH_TEST);

    glUniform1f(scale, 1.0f);
    float rotation_array[16] = {0.0};
    float q_axis_array[4] = {1.0, 0.0, 0.0, 0.0};
    rotation_transform(rotation_array, 0.2, 0.0, 0.0);
    // double x, y, theta = 0.01;
    double phi;
    double mouse[2] = {0};
    struct mouse_positions mouse_positions = {0, {0.0}, {0.0}, {0.0}};
    mouse_positions.active = 0;
    int w, h;
    float camera_vector[3] = {0.0, 0.0, 1.0};
    float axis[3] = {0.0, 1.0, 1.0};
    float rotation_axis[3] = {0.0};
    float q_rotation_axis[4] = {0.0};
    glfwSetScrollCallback(window, scroll_function);
    for (int i = 0; !glfwWindowShouldClose(window); i++) {
        phi = i/60.0;
        glClearColor(0.0, 0.0, 0.0, 1.0);
        rotation_transform(rotation_array, phi, phi/2, 0.0);
        // axis_rotation(q_axis_array, phi, axis);
        glUniform1f(scale, scroll_value);
        glUniform4f(
            q_axis, 
            q_axis_array[0], q_axis_array[1], 
            q_axis_array[2], q_axis_array[3]
            );
        glUniformMatrix4fv(rotation, 1, 0, rotation_array);
        /*
        int vertex_size = 6;
        int number_of_vertices = 8;
        for (int k = 0; k < number_of_vertices; k++) {
            x = vertices[vertex_size*k];
            y = vertices[vertex_size*k + 2];
            vertices[vertex_size*k] = (float)(cos(theta)*x - sin(theta)*y);
            vertices[vertex_size*k + 2] = (float)(sin(theta)*x + cos(theta)*y);
        }
        for (int k = 0; k < number_of_vertices; k++) {
            x = vertices[vertex_size*k + 1];
            y = vertices[vertex_size*k + 2];
            vertices[vertex_size*k + 1] = (float)(cos(theta)*x - sin(theta)*y);
            vertices[vertex_size*k + 2] = (float)(sin(theta)*x + cos(theta)*y);
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);*/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwGetCursorPos(window, &mouse[0], &mouse[1]);
        glfwGetFramebufferSize(window, &w, &h);
        // mouse_positions.curr[0] = mouse[0]/w - 0.5;
        // mouse_positions.curr[1] = 0.5 - mouse[1]/h;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            mouse_positions.curr[0] = mouse[0]/w - 0.5;
            mouse_positions.curr[1] = 0.5 - mouse[1]/h;
            if (!mouse_positions.active) {
                mouse_positions.active = 1;
                mouse_positions.prev[0] = mouse[0]/w - 0.5;
                mouse_positions.prev[1] = 0.5 - mouse[1]/h;
            } else {
                mouse_positions.change[0] = mouse_positions.curr[0]; 
                mouse_positions.change[0] -= mouse_positions.prev[0];
                mouse_positions.change[1] = mouse_positions.curr[1];
                mouse_positions.change[1] -= mouse_positions.prev[1];
                mouse_positions.change[2] = 0.0; 
                mouse_positions.prev[0] = mouse_positions.curr[0];
                mouse_positions.prev[1] = mouse_positions.curr[1]; 
            }
            if (mouse_positions.active) {
                if (
                    mouse_positions.change[0] != 0.0F || 
                    mouse_positions.change[1] != 0.0F) 
                    {
                    cross_product(rotation_axis, camera_vector, mouse_positions.change);
                    float angle = 15.0*sqrt(
                        mouse_positions.change[0]*mouse_positions.change[0] + 
                        mouse_positions.change[1]*mouse_positions.change[1]
                        );
                    axis_rotation(q_rotation_axis, angle, rotation_axis);
                    quaternion_multiply_by(q_axis_array, q_rotation_axis);
                    /*printf(
                        "%f, %f\n", 
                        mouse_positions.change[0], mouse_positions.change[1]);*/
                }
            }
        } else {
            mouse_positions.active = 0;
        }
        // glfwGetMousePos(&mouse[0], &mouse[1]);
    }

    // TODO: clean up memory and properly close things
    free((void *)v_source);
    free((void *)f_source);

    return 0;
}

