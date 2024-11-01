/* Numerical simulation of the Pauli equation in 3D.

References

Numerical method (centred second order difference in time):

 - Visscher, P. (1991). 
 A fast explicit algorithm for the time‐dependent Schrödinger equation.
 Computers in Physics, 5, 596-598.
 [https://doi.org/10.1063/1.168415](https://doi.org/10.1063/1.168415)

- Ira Moxley III, F. (2013).
 [Generalized finite-difference time-domain schemes 
  for solving nonlinear Schrödinger equations]
 (https://digitalcommons.latech.edu/cgi/
  viewcontent.cgi?article=1284&context=dissertations).
  Dissertation, 290. 

Explanation of the Pauli equation:

 - Wikipedia contributors. (2022, August 5).
 [Pauli equation](https://en.wikipedia.org/wiki/Pauli_equation).
 In Wikipedia, The Free Encyclopedia.

 - Shankar, R. (1994). Spin.
 In Principles of Quantum Mechanics, chapter 14. Springer.

 - Shankar, R. (1994). The Dirac Equation. 
 In Principles of Quantum Mechanics, chapter 20. Springer.

Finite difference stencils for discretizing the derivative terms of
the Pauli equation:

 - Fornberg, B. (1988).
 Generation of Finite Difference Formulas on Arbitrarily Spaced Grids.
 Mathematics of Computation, 51(184), 699-706.
 [https://doi.org/10.1090/S0025-5718-1988-0935077-0]
 (https://doi.org/10.1090/S0025-5718-1988-0935077-0)

*/
#include "pauli_leapfrog_3d.hpp"

// #include <OpenGL/OpenGL.h>
#include <GL/gl.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "render.hpp"
#include "interactor.hpp"
#include "quaternions.hpp"

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
// #include <OpenGL/OpenGL.h>
#include <vector>
#include "fft.h"
#include <iostream>
#include <ctime>
#include "bitmap.h"
#include "summation.h"
#include "render.hpp"
#include "vector_field_view_3d.hpp"
#include <GLES3/gl3.h>

#include "volume_render.hpp"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <functional>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"


// #include "imgui/imgui.h"
// #include "imgui/backends/imgui_impl_glfw.h"
// #include "imgui/backends/imgui_impl_opengl3.h"

static std::function <void()> loop;
#ifdef __EMSCRIPTEN__
static void main_loop() {
    loop();
}
#endif

static const double PI = 3.14159;

static struct SimulationParams {
    int nx = 100;
    int ny = 100;
    int nz = 100;
    double hbar = 1.0;
    double m = 1.0;
    double dt  = 0.1;
    double width = 100.0;
    double height = 100.0;
    double length = 100.0;
    double c = 137.036;
    double dx = 1.0;
    double dy = 1.0;
    double dz = 1.0;
    struct InitWavePacket {
        double a = 10.0;
        // double sx = 0.10, sy = 0.10, sz = 1.0;
        double sx = 0.07, sy = 0.07, sz = 0.07;
        double bx = 0.5, by = 0.5, bz = 0.5;
        double nx = 5.0, ny = 5.0, nz = 20.0;
    } init_wave_packet;
} sim_params;

static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

static Texture2DData get_texture_coordinate(
    int orientation_index, SimulationParams sim_params) {
    Texture2DData (* funcs[3])(
        double, double, int, int, int, int, 
        GLuint, GLuint, GLuint, GLuint
        ) = {&funcs3D::make_x, &funcs3D::make_y, &funcs3D::make_z};
    return funcs[orientation_index](
        0.0, 1.0, FLOAT, 
        sim_params.nx, sim_params.ny, sim_params.nz,
        GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR
        ).cast_to(COMPLEX2, X, NONE, X, NONE);
}

static Texture2DData color_phase_data(const Drawer view_proc,
                                      const Texture2DData &comp_data,
                                      SimulationParams sim_params) {
    Texture2DData col_data
        = funcs3D::zeroes(FLOAT4,
         sim_params.nx, sim_params.ny, sim_params.nz); 
    /* Texture2DData abs_data = (comp_data*conj(comp_data)
        ).cast_to(FLOAT4, NONE, NONE, NONE, X);*/
    auto comp_data2 = comp_data; // + abs_data;
    view_proc.draw(
        col_data,
        {
            {"tex", {&comp_data}},
        }
    );
    return col_data;
}



static Texture2DData get_initial_wavepacket(const Texture2DData &x, 
                                            const Texture2DData &y,
                                            const Texture2DData &z, 
                                            const SimulationParams &params) {
    auto imag_unit = std::complex<double>(0.0, 1.0);
    double a = sim_params.init_wave_packet.a;
    double sx = sim_params.init_wave_packet.sx;
    double sy = sim_params.init_wave_packet.sy;
    double sz = sim_params.init_wave_packet.sz;
    double bx = sim_params.init_wave_packet.bx;
    double by = sim_params.init_wave_packet.by;
    double bz = sim_params.init_wave_packet.bz;
    double nx = sim_params.init_wave_packet.nx;
    double ny = sim_params.init_wave_packet.ny;
    double nz = sim_params.init_wave_packet.nz;
    auto xb = x - bx;
    auto yb = y - by;
    auto zb = z - bz;
    std::cout << sx << std::endl;
    return a*exp(2.0*PI*imag_unit*(nx*xb + ny*yb + nz*zb))
        *exp(-0.5*(xb*xb/(sx*sx) + yb*yb/(sy*sy) + zb*zb/(sz*sz)));
    
}

int pauli_leapfrog_3d(Renderer *renderer) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    GLFWwindow *window = renderer->window;
    frame_id main_frame = renderer->main_frame;
    window_dimensions(window, &window_width, &window_height);

    /* sim_params.dx = sim_params.width/sim_params.nx;
    sim_params.dy = sim_params.height/sim_params.ny;
    sim_params.dz = sim_params.length/sim_params.nz;*/

    use_3d_texture(sim_params.nx, sim_params.ny, sim_params.nz);
    auto imag_unit = std::complex<double>(0.0, 1.0);

    // Create a texture that will store the Laplacian of psi
    auto laplacian_psi = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);

    // Laplacian program
    auto laplacian_command
         = DrawTexture2DData(Path("./shaders/laplacian/laplacian3d.frag"));
    auto view_proc = Drawer(Path("./shaders/view2.frag"));

    struct IVec2 tex_dimensions_2d 
        = get_2d_from_width_height_length(sim_params.nx, sim_params.ny, sim_params.nz);
    laplacian_command.set_ivec2_uniforms(
        {{"texelDimensions2D", tex_dimensions_2d}});
    laplacian_command.set_ivec3_uniforms({
        {"texelDimensions3D", 
            {sim_params.nx, sim_params.ny, sim_params.nz}},
    });
    laplacian_command.set_vec3_uniforms({
        {"dr", {(float)sim_params.dx, 
                (float)sim_params.dy,
                (float)sim_params.dz}},
        {"dimensions3D", 
             {(float)sim_params.nx,
              (float)sim_params.ny, 
              (float)sim_params.nz}},
    });

    auto gradient_procedure = Drawer(Path{"./shaders/gradient/gradient3d.frag"});

    auto h_psi_func = [&](Texture2DData &psi, Texture2DData &pot) {
        double hbar = sim_params.hbar;
        double m = sim_params.m;
        laplacian_command.draw(laplacian_psi, "tex", psi);
        return ((-hbar*hbar)/(2.0*m))*laplacian_psi + pot*psi;
    };

    Texture2DData gradient = Texture2DData(
        COMPLEX2, sim_params.nx, sim_params.ny);
    Texture2DData pot = Texture2DData(
        COMPLEX2, sim_params.nx, sim_params.ny
    );
    Texture2DData psi1 = Texture2DData(
        COMPLEX2, sim_params.nx, sim_params.ny
    );
    Texture2DData psi2 = Texture2DData(
        COMPLEX2, sim_params.nx, sim_params.ny
    );
    Texture2DData psi3 = Texture2DData(
        COMPLEX2, sim_params.nx, sim_params.ny
    );
    {
        auto x = get_texture_coordinate(0, sim_params);
        auto y = get_texture_coordinate(1, sim_params);
        auto z = get_texture_coordinate(2, sim_params);
        gradient = 0.0*x;
        pot = 4.0*((x-0.5)*(x-0.5) + (y-0.5)*(y-0.5) + (z-0.5)*(z-0.5));
        psi1 = get_initial_wavepacket(x, y, z, sim_params);
        psi2 = psi1 - (imag_unit*(0.5*sim_params.dt/sim_params.hbar))*h_psi_func(psi1, pot);
        psi3 = psi2;
    }

    auto vec_view = VectorFieldView3D(
        {window_width, window_height}, 
       {sim_params.nx, sim_params.ny, sim_params.nz});


    int view_program = make_quad_program("./shaders/view.frag");
    int copy_program = make_quad_program("./shaders/util/copy.frag");

    glViewport(0, 0, window_width, window_height);
    IVec2 view_dimensions {window_width, window_height};
    IVec3 render_dimensions {256, 256, 512};
    IVec3 sample_dimensions {sim_params.nx, sim_params.ny, sim_params.nz};
    auto vol = VolumeRender(view_dimensions, render_dimensions, sample_dimensions);

    int k = 0;
    bool exit_loop = false;

    Quaternion rotation {0.0, 0.0, 0.0, 1.0};
    Interactor interactor {window};

    // Initialize imgui
    bool show_controls_window = true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    #ifndef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 330 core");
    #else
    ImGui_ImplOpenGL3_Init("#version 300 es");
    #endif

    loop = [&] {

        use_3d_texture(sim_params.nx, sim_params.ny, sim_params.nz);
        int steps_per_frame = 20;
        for (int i = 0; i < steps_per_frame; i++) {
            psi3 = psi1 - (imag_unit*sim_params.dt/sim_params.hbar)
                            *h_psi_func(psi2, pot);
            swap(psi3, psi2);
            swap(psi1, psi3);
        }
        auto abs_psi1 = psi1*conj(psi1);
        gradient_procedure.draw(
            gradient, 
            {{"tex", {&abs_psi1}},
             {"orderOfAccuracy", {int(2)}},
             {"boundaryType", {int(0)}},
             {"staggeredMode", {int(0)}},
             {"index", {int{0}}},
             {"texelDimensions3D", 
              {IVec3 {sim_params.nx, sim_params.ny, sim_params.nz}}},
              {"texelDimensions2D", 
               {IVec2 {sim_params.nx*sim_params.nz, sim_params.ny}}},
               {"dr", {Vec3 {1.0, 1.0, 1.0}}},
               {"dimensions3D",
               {Vec3 {(float)sim_params.width, 
                      (float)sim_params.height, 
                      (float)sim_params.length}}}
             });
        gradient = gradient*0.25;
        auto psi_col_data = color_phase_data(view_proc, psi1, sim_params);
        // vec_view.render(conj(psi1)*psi1, j, 1.0, {0.0, 0.0, 0.0, 1.0});
        auto abs_psi_display = abs_psi1.cast_to(FLOAT4, X, NONE, NONE, X);
        auto pot_col_data = 0.015*pot.cast_to(FLOAT4, X, X, X, X);
        auto vol_display_data = psi_col_data + pot_col_data;
        glViewport(0, 0, window_width, window_height);
        // tex_copy(main_frame, gradient.get_frame_id());
        // Quaternion rot0 = {0.0, 1.0, 0.0, 1.0};
        // auto rot = rot0.normalized();
        double scroll = 2.0*Interactor::get_scroll()/25.0;
        auto vol_display = vol.render(vol_display_data, scroll, rotation);
        bind_quad(main_frame, copy_program);
        vol_display.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        
        glfwPollEvents();

        /* // Process input for ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Controls", &show_controls_window);
            ImGui::Text("WIP AND INCOMPLETE");
            ImGui::End();
            
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        */

        
        interactor.click_update(renderer);
        if (interactor.left_pressed()) {
            double angle = 4.0*interactor.get_mouse_abs_delta();
            auto vel = interactor.get_mouse_delta();
            Quaternion vel_q {.x=-vel.x, .y=-vel.y, .z=0.0, .w=0.0};
            Quaternion to_camera {.x=0.0, .y=0.0, .z=1.0, .w=0.0};
            Quaternion unorm_axis = vel_q*to_camera;
            Quaternion axis = unorm_axis.normalized();
            if (unorm_axis.length() > 0.0) {
                Quaternion r = rotator(angle, axis.x, axis.y, axis.z);
                rotation = rotation*r;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }

        k++;
        glfwSwapBuffers(window);
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop)
        loop();
    #endif

    return exit_status;
}