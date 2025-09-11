
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
    if (ImGui::SliderFloat("Scale", &params->brightness, 0.0, 20.0))
           s_sim_params_set(params->BRIGHTNESS, params->brightness);
    ImGui::Text("Domain dimensions");
    if (ImGui::SliderFloat("simulationDimensions3D[0]", &params->simulationDimensions3D.ind[0], 32.0, 512.0))
           s_sim_params_set(params->SIMULATION_DIMENSIONS3_D, params->simulationDimensions3D);
    if (ImGui::SliderFloat("simulationDimensions3D[1]", &params->simulationDimensions3D.ind[1], 32.0, 512.0))
           s_sim_params_set(params->SIMULATION_DIMENSIONS3_D, params->simulationDimensions3D);
    if (ImGui::SliderFloat("simulationDimensions3D[2]", &params->simulationDimensions3D.ind[2], 32.0, 512.0))
           s_sim_params_set(params->SIMULATION_DIMENSIONS3_D, params->simulationDimensions3D);
    if (ImGui::BeginMenu("Presets")) {
        if (ImGui::MenuItem( "exp(-0.5*x^2/(10.0)^2)*sin(z/4.0)*sin(y/4.0)/(z*y)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 0);
        if (ImGui::MenuItem( "20.0*exp(0.0-0.5*((x/(sx*10.0))^2 + (y/(sy*10.0))^2))"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 1);
        if (ImGui::MenuItem( "(x + i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 2);
        if (ImGui::MenuItem( "exp(-0.5*((x/(sx*10.0))^2 + (y/(sy*15.0))^2 + (z/(sz*10.0))^2))"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 3);
        if (ImGui::MenuItem( "a*sin(x/10)*sin(y/10)*sin(z/10)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 4);
        if (ImGui::MenuItem( "step(sqrt(x^2 + y^2 + z^2) - 80)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 5);
        if (ImGui::MenuItem( "1 - step(x - 30) - step(-x - 30)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 6);
        if (ImGui::MenuItem( "abs(cos(k*x*y*z^2/1500000))^100"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 7);
        if (ImGui::MenuItem( "log(abs(x/10))*log(abs(y/10))*log(abs(z/10))/10"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 8);
        if (ImGui::MenuItem( "cos(10*x*y*z/100000)^3"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 9);
        if (ImGui::MenuItem( "exp(-sqrt((x/5)^2 + (y/5)^2 + (z/5)^2))*(z + x)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 10);
        if (ImGui::MenuItem( "exp(-0.5*((x-x0)^2 + (y-y0)^2 + (z - z0)^2)/(s*15)^2)*exp(-i*(nx*x/50 + ny*y/50 + nz*z/50))"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 11);
        if (ImGui::MenuItem( "(x+ i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6*exp(-f*i*t)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 12);
        if (ImGui::MenuItem( "exp(-0.5*z^2/(sz*4)^2) - exp(-0.5*y^2/(sy*4)^2) - i*exp(-0.5*x^2/(sx*4)^2)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 13);
        if (ImGui::MenuItem( "(x+i*y)^18/(x^2 + y^2)*exp(-(x^2+y^2 + z^2)/100)*(z/depth)^16*exp(-i*f*t)"))
            s_selection_set(params->PRESET_FUNCTIONS_DROPDOWN, 14);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Visualization select")) {
        if (ImGui::MenuItem( "Volume render"))
            s_selection_set(params->VISUALIZATION_SELECT, 0);
        if (ImGui::MenuItem( "Three orthogonal planar slices"))
            s_selection_set(params->VISUALIZATION_SELECT, 1);
        if (ImGui::MenuItem( "Vector field"))
            s_selection_set(params->VISUALIZATION_SELECT, 2);
        ImGui::EndMenu();
    }
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Volume Render Controls");
    if (ImGui::SliderFloat("Alpha brightness", &params->alphaBrightness, 0.0, 10.0))
           s_sim_params_set(params->ALPHA_BRIGHTNESS, params->alphaBrightness);
    if (ImGui::SliderFloat("Color brightness", &params->colorBrightness, 0.0, 10.0))
           s_sim_params_set(params->COLOR_BRIGHTNESS, params->colorBrightness);
    if (ImGui::SliderFloat("Noise sampling strength", &params->noiseScale, 0.0, 1.5))
           s_sim_params_set(params->NOISE_SCALE, params->noiseScale);
    ImGui::Checkbox("Apply blur", &params->applyBlur);
    if (ImGui::SliderInt("Size", &params->blurSize, 0, 10))
            s_sim_params_set(params->BLUR_SIZE, params->blurSize);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Three Orthogonal Planar Slices Controls");
    ImGui::Text("Planar slices offsets (in normalized coordinates) for xy, yz, xz");
    if (ImGui::SliderFloat("planarNormCoordOffsets[0]", &params->planarNormCoordOffsets.ind[0], 0.0, 1.0))
           s_sim_params_set(params->PLANAR_NORM_COORD_OFFSETS, params->planarNormCoordOffsets);
    if (ImGui::SliderFloat("planarNormCoordOffsets[1]", &params->planarNormCoordOffsets.ind[1], 0.0, 1.0))
           s_sim_params_set(params->PLANAR_NORM_COORD_OFFSETS, params->planarNormCoordOffsets);
    if (ImGui::SliderFloat("planarNormCoordOffsets[2]", &params->planarNormCoordOffsets.ind[2], 0.0, 1.0))
           s_sim_params_set(params->PLANAR_NORM_COORD_OFFSETS, params->planarNormCoordOffsets);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Arrows Plot");

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
