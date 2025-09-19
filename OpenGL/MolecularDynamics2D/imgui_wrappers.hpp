
#include "parameters.hpp"

#ifndef _IMGUI_CONTROLS_
#define _IMGUI_CONTROLS_
using namespace sim_2d;

#include "gl_wrappers.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <functional>
#include <set>

#include "parameters.hpp"

static std::function<void(int, Uniform)> s_sim_params_set;
static std::function<void(int, int, std::string)> s_sim_params_set_string;
static std::function<Uniform(int)> s_sim_params_get;
static std::function<void(int, std::string, float)> s_user_edit_set_value;
static std::function<float(int, std::string)> s_user_edit_get_value;
static std::function<std::string(int)>
    s_user_edit_get_comma_separated_variables;
static std::function<void(int)> s_button_pressed;
static std::function<void(int, int)> s_selection_set;
static std::function<void(int, std::string, float)>
    s_sim_params_set_user_float_param;

static ImGuiIO global_io;
static std::map<int, std::string> global_labels;

void edit_label_display(int c, std::string text_content) {
    global_labels[c] = text_content;
}

void display_parameters_as_sliders(
    int c, std::set<std::string> variables) {
    std::string string_val = "[";
    for (auto &e: variables)
        string_val += """ + e + "", ";
    string_val += "]";
    string_val 
        = "modifyUserSliders(" + std::to_string(c) + ", " + string_val + ");";
    // TODO
}

void start_gui(void *window) {
    bool show_controls_window = true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void imgui_controls(void *void_params) {
    SimParams *params = (SimParams *)void_params;
    for (auto &e: global_labels)
        params->set(e.first, 0, e.second);
    ImGui::Text("Frames/s");
    ImGui::Text("Total energy (ought to be conserved)");
    ImGui::Text("Cummulative total energy drift error (percentage of initial energy)");
    if (ImGui::SliderInt("Steps per frame", &params->stepsPerFrame, 0, 50))
            s_sim_params_set(params->STEPS_PER_FRAME, params->stepsPerFrame);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Time Step Configuration");
    if (ImGui::SliderFloat("Requested time step", &params->requestedDt, 0.0, 1e-07))
           s_sim_params_set(params->REQUESTED_DT, params->requestedDt);
    if (ImGui::SliderFloat("Adaptive time step reduction agressiveness", &params->limitTimeStepAgressiveness, 100.0, 1000.0))
           s_sim_params_set(params->LIMIT_TIME_STEP_AGRESSIVENESS, params->limitTimeStepAgressiveness);
    ImGui::Text("Actual time step (dtLabel)");
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Lennard-Jones Configuration");
    ImGui::Text("σ (related to particle size) = 0.0075");
    if (ImGui::SliderFloat("ε (controls strength of inter-particle interaction)", &params->epsilon, 20000.0, 20000000.0))
           s_sim_params_set(params->EPSILON, params->epsilon);
    ImGui::Text("--------------------------------------------------------------------------------");
    if (ImGui::SliderFloat("Gravitational force", &params->gForce, -20000000.0, 0.0))
           s_sim_params_set(params->G_FORCE, params->gForce);
    if (ImGui::SliderFloat("Particle Brightness", &params->particleBrightness, 0.0, 1.0))
           s_sim_params_set(params->PARTICLE_BRIGHTNESS, params->particleBrightness);
    ImGui::Text("Simulation domain");
    ImGui::Text("0 ≤ x < 1.7");
    ImGui::Text("0 ≤ y < 1.7");
    if (ImGui::BeginMenu("Number of particles:")) {
        if (ImGui::MenuItem( "64"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 0);
        if (ImGui::MenuItem( "128"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 1);
        if (ImGui::MenuItem( "256"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 2);
        if (ImGui::MenuItem( "512"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 3);
        if (ImGui::MenuItem( "1024"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 4);
        if (ImGui::MenuItem( "2048"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 5);
        if (ImGui::MenuItem( "4096"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 6);
        if (ImGui::MenuItem( "8192"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 7);
        if (ImGui::MenuItem( "16384"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 8);
        if (ImGui::MenuItem( "32768"))
            s_selection_set(params->PARTICLE_COUNT_SELECTOR, 9);
        ImGui::EndMenu();
    }
    ImGui::Checkbox("Display cells with particles", &params->showCellsByParticleCount);
    if (ImGui::BeginMenu("For each particle, select the number of neighbour cells to traverse in order to find its non-zero interaction with other particles: ")) {
        if (ImGui::MenuItem( "3"))
            s_selection_set(params->NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION_SELECTOR, 0);
        if (ImGui::MenuItem( "8"))
            s_selection_set(params->NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION_SELECTOR, 1);
        ImGui::EndMenu();
    }

}

bool outside_gui() {{
    return !global_io.WantCaptureMouse;
}}

void display_gui(void *data) {{
    global_io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    bool val = true;
    ImGui::Begin("Controls", &val);
    ImGui::Text("WIP AND INCOMPLETE");
    imgui_controls(data);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}}

#endif
