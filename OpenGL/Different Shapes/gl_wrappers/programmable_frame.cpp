#include "gl_wrappers.h"

namespace gl_wrappers {

    void ProgrammableFrame::set_program(GLuint program) {
        this->program = program;
        glUseProgram(program);
    }

    void ProgrammableFrame::set_int_uniform(const char *name, int val) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1i(loc, val);
    }

    void ProgrammableFrame::set_int_uniform(const std::string &name, int val) {
        GLuint loc = glGetUniformLocation(program, name.c_str());
        glUniform1i(loc, val);
    }

    void ProgrammableFrame::set_int_uniforms(const std::map<std::string, int>
                                             &uniforms) {
        for (const auto &uniform: uniforms) {
            set_int_uniform(uniform.first, uniform.second);
        }
    }

    void ProgrammableFrame::set_float_uniform(const char *name, float val) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1f(loc, val);
    }

    void ProgrammableFrame::set_float_uniform(const std::string &name,
                                              float val) {
        GLuint loc = glGetUniformLocation(program, name.c_str());
        glUniform1f(loc, val);
    }

    void ProgrammableFrame::set_float_uniforms(const std::map<std::string,
                                               double> &uniforms) {
        for (const auto &uniform: uniforms) {
            set_float_uniform(uniform.first, uniform.second);
        }
    }

    void ProgrammableFrame::set_vec2_uniform(const char *name, const float *v) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2f(loc, v[0], v[1]);
    }

    void ProgrammableFrame::set_vec2_uniform(const char *name,
                                             const std::vector <float> &v) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2f(loc, v[0], v[1]);
    }

    void ProgrammableFrame::set_vec2_uniform(const char *name,
                                             float v0, float v1) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2f(loc, v0, v1);
    }

    void ProgrammableFrame::set_vec2_uniforms(const std::map<std::string, 
                                                             std::pair<float, 
                                                                       float> 
                                                            > &uniforms) {
        for (auto &uniform: uniforms) {
            set_vec2_uniform(uniform.first.c_str(), 
                             uniform.second.first, uniform.second.second);
        } 

    }

    void ProgrammableFrame::set_vec3_uniform(const char *name,
                                             const std::vector<float> &v) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3f(loc, v[0], v[1], v[2]);
    }

    void ProgrammableFrame::set_vec3_uniform(const char *name,
                                             float v0, float v1, float v2) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3f(loc, v0, v1, v2);
    }

    void ProgrammableFrame::set_vec3_uniform(const char *name,
                                             const float *v) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3f(loc, v[0], v[1], v[2]);
    }

    void ProgrammableFrame::set_vec4_uniform(const char *name,
                                             const std::vector<float> &v) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4f(loc, v[0], v[1], v[2], v[3]);
    }

    void ProgrammableFrame::set_vec4_uniform(const char *name,
                                             float v0, float v1,
                                             float v2, float v3) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4f(loc, v0, v1, v2, v3);
    }

    void ProgrammableFrame::set_vec4_uniform(const char *name,
                                             const float *v) {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4f(loc, v[0], v[1], v[2], v[3]);
    }

}
