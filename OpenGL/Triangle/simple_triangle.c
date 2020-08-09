#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>


/*
* Colour struct, whose
* members are red, green, and blue,
* which are ints from 0 to 255.
*/
struct Colour {
    int red;
    int green;
    int blue;
};


/*
* Get the angle from the positive x-axis of
* a vector with x and y components. For positive
* values of y the angle is positive while for
* negative values of y the angle is negative.
*/
double get_phase(double x, double y) {
    double pi = 3.14159265359;
    if (y >= 0.0) {
        if (x > 0.0) {
            return atan(y/x);
        } else {
            return atan(-x/y) + pi/2.0; 
        }
    } else {
        if (x > 0.0) {
            return -atan(-y/x);
        } else {
            return -atan(x/y) - pi/2.0;
        }
    }
}

/*
* Turn an x and y value to a colour.
* Reference:
* Wikipedia contributors. (2020). Hue. 
* In Wikipedia, The Free Encyclopedia.
* https://en.wikipedia.org/wiki/Hue
*/
void point_to_colour(struct Colour *col, double x, double y) {
    double pi = 3.141592653589793;
    double arg_val = get_phase(x, y);
    int max_col = 255;
    int min_col = 50;
    int col_range = max_col - min_col;
    if (arg_val <= pi/3 && arg_val >= 0) {
        col->red = max_col;
        col->green = min_col + (int)(col_range*arg_val/(pi/3));
        col->blue = min_col;
    } else if (arg_val > pi/3 && arg_val <= 2*pi/3){
        col->red = max_col - (int)(col_range*(arg_val - pi/3)/(pi/3));
        col->green = max_col;
        col->blue = min_col;
    } else if (arg_val > 2*pi/3 && arg_val <= pi){
        col->red = min_col;
        col->green = max_col;
        col->blue = min_col + (int)(col_range*(arg_val - 2*pi/3)/(pi/3));
    } else if (arg_val < 0.0 && arg_val > -pi/3){
        col->red = max_col;
        col->blue = min_col - (int)(col_range*arg_val/(pi/3));
        col->green = min_col;
    } else if (arg_val <= -pi/3 && arg_val > -2*pi/3){
        col->red = max_col + (int)(col_range*(arg_val + pi/3)/(pi/3));
        col->blue = max_col;
        col->green = min_col;
    } else if (arg_val <= -2*pi/3 && arg_val >= -pi){
        col->red = min_col;
        col->blue = max_col;
        col->green = min_col - (int)(col_range*(arg_val + 2*pi/3)/(pi/3));
    }
    else {
        col->red = min_col;
        col->green = max_col;
        col->blue = max_col;
    }
}

/*
* Given a filename, get its file contents.
*/
char *get_file_contents(char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *file_buff = malloc(file_size + 1);
    if (file_buff == NULL) {
        perror("malloc");
        fclose(f);
        return NULL;
    }
    fread(file_buff, file_size, 1, f);
    file_buff[file_size] = '\0';
    fclose(f);
    return file_buff;
}


int main() {

    // Get contents of shader files.
    char *file_contents = get_file_contents("vertex.frag");
    if (file_contents == NULL) {
        fprintf(stderr, "Unable to open vertex shader");
        exit(1);
    }
    const char *v_source = file_contents;
    char *file_contents2 = get_file_contents("fragment.frag");
    if (file_contents2 == NULL) {
        fprintf(stderr, "Unable to open fragment shader");
        exit(1);
    }
    const char *f_source = file_contents2;

    GLchar buf[1024];
    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version. This is NECESSARY
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version. This is NECESSARY 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ???
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // ???
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow *window = glfwCreateWindow(600, 600, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; // This is NECESSARY
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error.\n");
        exit(1);
    }
    
    // Vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vertex buffer object
    GLuint vbo;
    float vertices[6] = {0.0, 0.25, 0.25, -0.25, -0.25, -0.25};
    glGenBuffers(1, &vbo);  
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader.
    GLuint vs_ref = glCreateShader(GL_VERTEX_SHADER);
    if (vs_ref == 0) {
        fprintf(stderr, "unable to "
                "create vertex shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    glShaderSource(vs_ref, 1, &v_source, NULL);
    glCompileShader(vs_ref);
    GLint v_status;
    glGetShaderiv(vs_ref, GL_COMPILE_STATUS, &v_status);
    glGetShaderInfoLog(vs_ref, 512, NULL, buf);
    if (v_status == GL_TRUE) {
        fprintf(stdout,
                "%s\n%s", "Vertex shader compiled successfully.", buf);
    } else {
        fprintf(stderr, 
                "%s\n%s", "Vertex shader compilation failed:", buf);
    }

    // Create and compile the fragment shader.
    GLuint fs_ref = glCreateShader(GL_FRAGMENT_SHADER);
    if (fs_ref == 0) {
        fprintf(stderr, "Error: unable to "
                "create fragment shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    glShaderSource(fs_ref, 1, &f_source, NULL);
    glCompileShader(fs_ref);
    GLint f_status;
    glGetShaderiv(fs_ref, GL_COMPILE_STATUS, &f_status);
    glGetShaderInfoLog(fs_ref, 512, NULL, buf);
    if (f_status == GL_TRUE) {
        fprintf(stdout, 
                "%s\n%s", "Fragment shader compiled successfully.", buf);
    } else {
        fprintf(stderr, 
                "%s\n%s", "Fragment shader compilation failed:", buf);
    }

    // Create the shader program and attach the shaders.
    GLuint program = glCreateProgram();
    glAttachShader(program, vs_ref);
    glAttachShader(program, fs_ref);
    glBindFragDataLocation(program, 0, "colour"); 
    glLinkProgram(program);
    glUseProgram(program);

    GLint pos_attrib = glGetAttribLocation(program, "position");
    // first parameter reference to the input
    // second parameter number of arguments
    // third parameter the type of each component
    // fourth parameter whether input should be normalized
    // fifth parameter number of bytes between each position in the array
    // sixth parameter the offset of the attribute
    glEnableVertexAttribArray(pos_attrib);
    // Specifies how the data from the input is retrieved from the array.
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint col = glGetUniformLocation(program, "dynColour");
    GLint rotation = glGetUniformLocation(program, "rotation");
    struct Colour colour = {0.0};
    float rotation_array[16] = {0.0};
    double x, y, theta = 0.05;
    double phi;
    for (int i = 0; !glfwWindowShouldClose(window); i++) {
        phi = i/120.0;
        glClearColor(0.0, 0.0, 0.0, 1.0);
        point_to_colour(&colour, cos(phi), sin(phi));
        rotation_array[0] = cos(phi);
        rotation_array[1] = -sin(phi);
        rotation_array[4] = cos(phi);
        rotation_array[5] = sin(phi);
        rotation_array[10] = 1.0f;
        rotation_array[15] = 1.0f;
        glUniformMatrix4fv(rotation, 1, 0, rotation_array);
        glUniform3f(col, (float)(colour.red/255.0), 
                    (float)(colour.green/255.0), (float)(colour.blue/255.0));
        for (int k = 0; k < 3; k++) {
            x = vertices[2*k];
            y = vertices[2*k+1];
            vertices[2*k] = (float)(cos(theta)*x - sin(theta)*y);
            vertices[2*k + 1] = (float)(sin(theta)*x + cos(theta)*y);
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // TODO: clean up memory and properly close things.


    return 0;
}

