#include "dirac_splitstep_3d.hpp"

// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
#include "vector_field_view_3d.hpp"
#include "render.hpp"
#include "interactor.hpp"
#include "quaternions.hpp"
// #include <OpenGL/OpenGL.h>
#include <vector>
#include "fft.h"
#include <iostream>
#include <ctime>
#include "bitmap.h"
#include "summation.h"
#include <GLES3/gl3.h>
#include <time.h>
#include <cmath>

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <functional>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "volume_render.hpp"

static std::function <void()> loop;
#ifdef __EMSCRIPTEN__
static void main_loop() {
    loop();
}
#endif

static const double PI = 3.14159;

static struct SimulationParams {
    int nx = 64;
    int ny = 64;
    int nz = 64;
    double hbar = 1.0;
    float m = 1.0;
    double width = 2.0;
    double height = 2.0;
    double length = 2.0;
    double c = 137.036;
    double dx() {
        return this->width/float(this->nx);
    }
    double dy() {
        return this->height/float(this->ny);
    }
    double dz() {
        return this->length/float(this->nz);
    }
    float max_time_step() {
        float dx = std::min(this->dx(), std::min(this->dy(), this->dz()));
        return dx/this->c;
    }
    float dt = max_time_step();
    float vec_length = 0.1;
    float brightness = 1.0;
    int steps_per_frame = 1;
    int representation = 0;
    int display_option = 1;
    float fps = 0.0;
    IVec3 texel_dimensions3d {this->nx, this->ny, this->nz};
    Vec3 dimensions3d {float(this->width), 
                       float(this->height), 
                       float(this->length)};
    bool init_new_wavepacket = false;
    struct InitWavePacket {
        double a = 10.0;
        double sx = 0.07, sy = 0.07, sz = 0.07;
        // double sx = 0.25, sy = 0.25, sz = 0.25;
        float bx = 0.5, by = 0.5, bz = 0.5;
        int nx = 20, ny = 20, nz = 20;
    } init_wave_packet;
} sim_params;

static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    double ns_in_ms = 1000000.0;
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/ns_in_ms;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/ns_in_ms;
}

/* Numerically solve the Dirac equation in 3D using the split-operator method.

References:

Dirac Equation:

 - Wikipedia contributors. (2023, February 6). 
   Dirac equation. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Dirac_equation

 - Wikipedia contributors. (2023, February 25).
   Dirac spinor. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Dirac_spinor

 - Shankar, R. (1994). The Dirac Equation. 
   In Principles of Quantum Mechanics, chapter 20. Springer.

 - Peskin, M., Schroeder, D. (1995). The Dirac Field.
   In An Introduction to Quantum Field Theory, chapter 3.
   CRC Press.

Split-Operator Method:

 - James Schloss. The Split-Operator Method.
   In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html
 
 - Wikipedia contributors. (2023, January 25). 
   Split-step method. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method

 - Bauke, H., Keitel, C. (2011).
   Accelerating the Fourier split operator method 
   via graphics processing units.
   Computer Physics Communications, 182(12), 2454-2463. 
   https://doi.org/10.1016/j.cpc.2011.07.003

Spin and Pauli Spin Matrices:

 - Shankar, R. (1994). Spin.
   In Principles of Quantum Mechanics, chapter 14. Springer.
 
 - Wikipedia contributors. (2023, March 6). 
   Pauli matrices. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Pauli_matrices

*/
static void time_step(const Texture2DData &u, const Texture2DData &v,
                      const Texture2DData &potential1,
                      const Texture2DData &potential2,
                      Drawer &kinetic_prop_drawer,
                      Drawer &spatial_prop_drawer,
                      const struct SimulationParams &sim_params) {
    // Define intermediary variables.
    auto u1 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    auto v1 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    auto u3 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    auto v3 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    // First pass for the position space propagator
    spatial_prop_drawer.draw(
        u1,
        {
            {"dt", {float(0.5*sim_params.dt)}},
            {"c", {float(sim_params.c)}},
            {"hbar", {float(sim_params.hbar)}},
            {"uTex", {&u}},
            {"vTex", {&v}},
            {"potentialTex", {&potential1}},
            {"spinorIndex", {int(0)}},
            {"representation", {sim_params.representation}},
        }
    );
    spatial_prop_drawer.draw(
        v1,
        {
            {"dt", {float(0.5*sim_params.dt)}},
            {"c", {float(sim_params.c)}},
            {"hbar", {float(sim_params.hbar)}},
            {"uTex", {&u}},
            {"vTex", {&v}},
            {"potentialTex", {&potential1}},
            {"spinorIndex", {int(1)}},
            {"representation", {sim_params.representation}},
        }
    );
    // Fourier transform to momentum space
    auto u2 = funcs3D::fft(u1);
    auto v2 = funcs3D::fft(v1);
    // Apply the kinetic propagator
    kinetic_prop_drawer.draw(
        u3,
        {
            {"numberOfDimensions", {int(3)}},
            {"texelDimensions3D", {sim_params.texel_dimensions3d}},
            {"dimensions3D", {sim_params.dimensions3d}},
            {"uTex", {&u2}},
            {"vTex", {&v2}},
            {"dt", {float(sim_params.dt)}},
            {"m", {float(sim_params.m)}},
            {"c", {float(sim_params.c)}},
            {"hbar", {float(sim_params.hbar)}},
            {"spinorIndex", {int(0)}},
            {"representation", {sim_params.representation}},
        }
    );
    kinetic_prop_drawer.draw(
        v3,
        {
            {"numberOfDimensions", {int(3)}},
            {"texelDimensions3D", {sim_params.texel_dimensions3d}},
            {"dimensions3D", {sim_params.dimensions3d}},
            {"uTex", {&u2}},
            {"vTex", {&v2}},
            {"dt", {float(sim_params.dt)}},
            {"m", {float(sim_params.m)}},
            {"c", {float(sim_params.c)}},
            {"hbar", {float(sim_params.hbar)}},
            {"spinorIndex", {int(1)}},
            {"representation", {sim_params.representation}},
        }
    );
    // Inverse fourier transform back to position space
    auto u4 = funcs3D::ifft(u3);
    auto v4 = funcs3D::ifft(v3);
    spatial_prop_drawer.draw(
        u,
        {
            {"dt", {float(0.5*sim_params.dt)}},
            {"c", {float(sim_params.c)}},
            {"hbar", {float(sim_params.hbar)}},
            {"uTex", {&u4}},
            {"vTex", {&v4}},
            {"potentialTex", {&potential2}},
            {"spinorIndex", {int(0)}},
            {"representation", {sim_params.representation}},
        }
    );
    spatial_prop_drawer.draw(
        v,
        {
            {"dt", {float(0.5*sim_params.dt)}},
            {"c", {float(sim_params.c)}},
            {"hbar", {float(sim_params.hbar)}},
            {"uTex", {&u4}},
            {"vTex", {&v4}},
            {"potentialTex", {&potential2}},
            {"spinorIndex", {int(1)}},
            {"representation", {sim_params.representation}},
        }
    );
};

static void slice_of_3d(Drawer &drawer, int orientation, int slice,
                        IVec2 slice_tex_dimensions,
                        IVec2 input_tex_dimensions_2d,
                        IVec3 input_tex_dimensions_3d,
                        const Texture2DData &out,
                        const Texture2DData &in) {
    drawer.draw(
        out,
        {
            {"orientation", {int(orientation)}},
            {"slice", {int(slice)}},
            {"sliceTexelDimensions2D", {slice_tex_dimensions}},
            {"inputTexelDimensions2D", {input_tex_dimensions_2d}},
            {"inputTexelDimensions3D", {input_tex_dimensions_3d}},
            {"tex", {&in}}
        }
    );
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

void get_initial_momentum_wavepacket(
    Texture2DData &u,
    Texture2DData &v,
    std::complex<float> u_p,
    std::complex<float> d_p,
    std::complex<float> u_n,
    std::complex<float> d_n,
    const Drawer &drawer,
    const SimulationParams &params
) {
    auto p = [](int i, float len) -> double {
        return 2.0*3.14159*float(i)/len;
    };
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
    Vec3 position {.x=float(bx*sim_params.width),
                   .y=float(by*sim_params.height),
                   .z=float(bz*sim_params.length)};
    Vec3 momentum {.x=float(p(int(nx), sim_params.width)), 
                   .y=float(p(int(ny), sim_params.height)),
                   .z=float(p(int(nz), sim_params.length))};
    Vec3 sigma {.x=float(p(int(0.15*sim_params.nx), sim_params.width)),
                .y=float(p(int(0.15*sim_params.ny), sim_params.height)),
                .z=float(p(int(0.15*sim_params.nz), sim_params.length))
                };
    auto w = funcs3D::zeroes(
        FLOAT4, 
        sim_params.nx, sim_params.ny, sim_params.nz);
    for (auto &i: {0, 1}) {
        drawer.draw(
            w,
            {
                {"dimensions", 
                {Vec3 {float(sim_params.width), 
                        float(sim_params.height),
                        float(sim_params.length)}}},
                {"texelDimensions", 
                {IVec3 {sim_params.nx, sim_params.ny, sim_params.nz}}},
                {"position", {position}},
                {"momentum", {momentum}},
                {"sigma", {sigma}},
                {"m", {float(sim_params.m)}},
                {"c", {float(sim_params.c)}},
                {"spinorIndex", {int(i)}},
                {"representation", {int(sim_params.representation)}},
                {"uPosECoeff", {Vec2 {u_p.real(), u_p.imag()}}},
                {"dPosECoeff", {Vec2 {d_p.real(), d_p.imag()}}},
                {"uNegECoeff", {Vec2 {u_n.real(), u_n.imag()}}},
                {"dNegECoeff", {Vec2 {d_n.real(), d_n.imag()}}},
            }
        );
        float factor = 0.05*float(sim_params.nx*sim_params.ny*sim_params.nz);
        if (i == 0)
            u = funcs3D::ifft(factor*w);
        else
            v = funcs3D::ifft(factor*w);
    }
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
    return a*exp(2.0*PI*imag_unit*(nx*xb + ny*yb + nz*zb))
        *exp(-0.5*(xb*xb/(sx*sx) + yb*yb/(sy*sy) + zb*zb/(sz*sz)));
    
}

int dirac_splitstep_3d(Renderer *renderer) {

    GLFWwindow *window = renderer->window;
    int main_frame = renderer->main_frame;

    int exit_status = 0;

    // Get the screen dimensions
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);

    sim_params.dimensions3d.ind[0] = sim_params.width;
    sim_params.dimensions3d.ind[1] = sim_params.height;
    sim_params.dimensions3d.ind[2] = sim_params.length;

    sim_params.texel_dimensions3d.ind[0] = sim_params.nx;
    sim_params.texel_dimensions3d.ind[1] = sim_params.ny;
    sim_params.texel_dimensions3d.ind[2] = sim_params.nz;

    auto imag_unit = std::complex<double>(0.0, 1.0);

    auto kinetic_procedure 
        = Drawer(Path("./shaders/dirac/split-step-momentum3d.frag"));
    auto spatial_procedure 
        = Drawer(Path("./shaders/dirac/split-step-spatial.frag"));
    auto current_procedure = Drawer(Path("./shaders/dirac/current.frag"));
    auto pseudocurrent_procedure 
        = Drawer(Path("./shaders/dirac/pseudocurrent.frag"));
    auto scalar_procedure = Drawer(Path("./shaders/dirac/scalar.frag"));
    /* auto pseudoscalar_procedure 
        = Drawer(Path("./shaders/dirac-pseudoscalar.frag"));*/
    auto slice_of_3d_procedure
        = Drawer(
            Path("./shaders/planes-projection-render/slice-of-3d.frag"));
    auto wavepacket_drawer
        = Drawer(Path("./shaders/dirac/init-momentum-wavepacket.frag"));


    glViewport(0, 0, sim_params.nx*sim_params.nz, sim_params.ny);

    auto x = get_texture_coordinate(0, sim_params);
    auto y = get_texture_coordinate(1, sim_params);
    auto z = get_texture_coordinate(2, sim_params);

    auto pot 
        = 0.0*100.0*powf(64.0F/(float)sim_params.nz, 2.0)
            *((x - 0.5)*(x - 0.5) 
              + (y - 0.5)*(y - 0.5) 
              + (z - 0.5)*(z - 0.5)
            ).cast_to(FLOAT4, NONE, NONE, NONE, X);
    auto u = get_initial_wavepacket(x, y, z, sim_params);
    auto v = 0.0*u;

    auto j = funcs3D::zeroes(
        FLOAT4, 
        sim_params.nx, sim_params.ny, sim_params.nz);
    auto scalar = funcs3D::zeroes(FLOAT, 
        sim_params.nx, sim_params.ny, sim_params.nz);

    auto vec_view = VectorFieldView3D(
        {window_width, window_height}, 
        {sim_params.nx, sim_params.ny, sim_params.nz});

    /* auto px = funcs3D::fftshift(2.0*PI*x);
    auto py = funcs3D::fftshift(2.0*PI*y);
    auto pz = funcs3D::fftshift(2.0*PI*z);*/

    int view_program = make_quad_program("./shaders/view.frag");
    int copy_program = make_quad_program("./shaders/util/copy.frag");

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
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    #ifndef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 330 core");
    #else
    ImGui_ImplOpenGL3_Init("#version 300 es");
    #endif

    // int show_controls_window = 1;

    char text[500] = {'\0',};

    auto vol = VolumeRender(
        {{{window_width, window_height}}}, 
        {{{128, 128, 64}}},
        {{{sim_params.nx, sim_params.ny, sim_params.nz}}}
        );
    // char a;
    // std::cin >> a;

    struct timespec frame_times[2] = {{}, {}};
    // #ifndef __EMSCRIPTEN__
    clock_gettime(CLOCK_MONOTONIC_RAW, &frame_times[0]);
    // #endif
    loop = [&] {

        glViewport(0, 0, sim_params.nx*sim_params.nz, sim_params.ny);
        for (int i = 0; i < sim_params.steps_per_frame; i++) {
            if (!sim_params.init_new_wavepacket)
                time_step(u, v, 
                          pot, pot, 
                          kinetic_procedure, spatial_procedure,
                          sim_params);
            /* time_step(u, v, pot, pot, 
                      kinetic_prop_drawer, spatial_prop_drawer, sim_params);*/
        }
        if (sim_params.init_new_wavepacket) {
            get_initial_momentum_wavepacket(u, v, 
                {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
                 wavepacket_drawer, sim_params);
            get_initial_momentum_wavepacket(u, v,
                {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, 
                wavepacket_drawer, sim_params);
            // u = get_initial_wavepacket(x, y, z, sim_params);
            // v = 0.0*u;
            sim_params.init_new_wavepacket = 0;
        }

        current_procedure.draw(
            j, {
                {"sigmaX", {Vec4 {0.0, 0.0, 1.0, 0.0}}},
                {"sigmaY", {Vec4 {0.0, 0.0, 0.0, -1.0}}},
                {"sigmaZ", {Vec4 {1.0, -1.0, 0.0, 0.0}}},
                {"representation", {sim_params.representation}},
                {"uTex", {&u}}, 
                {"vTex", {&v}}
                });
        scalar_procedure.draw(
            scalar, {
                {"representation", {sim_params.representation}},
                {"uTex", {&u}}, 
                {"vTex", {&v}}
                });
        // puts("This is reached.");
        auto scalar_alpha 
            = sqrt(scalar*scalar).cast_to(FLOAT4, NONE, NONE, NONE, X);
        auto scalar_red 
            = max(scalar.cast_to(FLOAT4, X, NONE, NONE, NONE), 0.0);
        // puts("This is reached.");
        auto scalar_blue 
            = max(0.0 - scalar.cast_to(FLOAT4, NONE, NONE, X, NONE), 0.0);
        auto scalar_vec_display = scalar_red + scalar_blue + scalar_alpha;
        auto scalar_vol_display = scalar_red + scalar_blue + scalar_alpha;
        auto j0 = j.cast_to(FLOAT4, W, W, W, W);
        /* auto ones = funcs3D::zeroes(FLOAT, 
            sim_params.nx, sim_params.ny, sim_params.nz) + 1.0;
        ones = ones.cast_to(FLOAT4, NONE, NONE, NONE, X);
        auto j0_2 = j0.cast_to(FLOAT4, X, NONE, NONE, X);*/
        double scroll = 2.0*Interactor::get_scroll()/25.0;
        // std::cout << scroll << std::endl;
        enum {SHOW_VECTOR_FIELD=0, VOL_DISPLAY=1, SLICE_XY=2, SLICE_ZY=3, SLICE_XZ=4};
        if (sim_params.display_option == SHOW_VECTOR_FIELD) {
            auto vec_display = vec_view.render(
            scalar_vec_display*sim_params.brightness, 
            sim_params.vec_length*j, scroll, rotation);
            glViewport(0, 0, window_width, window_height);
            bind_quad(main_frame, copy_program);
            vec_display.set_as_sampler2D_uniform("tex");
            draw_unbind_quad();
        } else if (sim_params.display_option == VOL_DISPLAY) {
            auto vol_display = vol.render(
            scalar_vol_display*sim_params.brightness, scroll, rotation);
            auto debug_grad = 10.0*vol.debug_get_grad_half_precision();
            auto debug_vol = 10.0*vol.debug_get_vol_half_precision();
            auto debug_sample_grad = vol.debug_get_sample_grad();
            auto debug_sample_vol = sim_params.brightness*vol.debug_get_sample_vol();
            glViewport(0, 0, window_width, window_height);
            bind_quad(main_frame, copy_program);
            // set_sampler2D_uniform("tex", vol_display.get_frame_id());
            set_sampler2D_uniform("tex", vol_display.get_frame_id());
            // vol_display.set_as_sampler2D_uniform("tex");
            draw_unbind_quad();
        } else if (sim_params.display_option == SLICE_XY 
                    || sim_params.display_option == SLICE_XZ
                    || sim_params.display_option == SLICE_ZY) {
            auto xy_slice
                 = funcs2D::zeroes(FLOAT4, sim_params.nx, sim_params.ny);
            glViewport(0, 0, sim_params.nx, sim_params.ny);
            slice_of_3d(slice_of_3d_procedure, sim_params.display_option - 2, 64/2, 
                        {sim_params.nx, sim_params.ny},
                        {sim_params.nx*sim_params.nz, sim_params.ny},
                        {sim_params.nx, sim_params.ny, sim_params.nz},
                        xy_slice,
                        scalar_vol_display);
            glViewport(0, 0, window_width, window_height);
            bind_quad(main_frame, copy_program);
            set_sampler2D_uniform("tex", xy_slice.get_frame_id());
            draw_unbind_quad();
        }

        glfwPollEvents();

         // Process input for ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Controls", &show_controls_window);
            ImGui::Text("WIP AND INCOMPLETE");
            ImGui::SliderFloat("Mass", &sim_params.m, 0.0, 2.0);
            ImGui::SliderFloat(
                "Time Step", &sim_params.dt, 
                -sim_params.max_time_step(), sim_params.max_time_step(), 
                "%g");
            ImGui::SliderInt(
                "Steps/Frame", &sim_params.steps_per_frame, 0, 10);
            ImGui::SliderFloat(
                "Vector Length", &sim_params.vec_length, 0.0, 2.0);
            ImGui::SliderFloat("Brightness",
                &sim_params.brightness, 0.0, 2.0);
            int s = 0;
            s += ImGui::SliderInt("nx", 
                &sim_params.init_wave_packet.nx, -20, 20);
            s += ImGui::SliderInt("ny", 
                &sim_params.init_wave_packet.ny, -20, 20);
            s += ImGui::SliderInt("nz", 
                &sim_params.init_wave_packet.nz, -20, 20);
            s += ImGui::SliderFloat("rx", 
                &sim_params.init_wave_packet.bx, 0.0, 1.0);
            s += ImGui::SliderFloat("ry", 
                &sim_params.init_wave_packet.by, 0.0, 1.0);
            s += ImGui::SliderFloat("rz", 
                &sim_params.init_wave_packet.bz, 0.0, 1.0);
            sim_params.init_new_wavepacket = s;

            ImGui::InputText("Text input", (char *)&text[0], 500);
            // ImGui::Checkbox("Show Volume Render", (bool *)&sim_params.display_option);
            if (ImGui::BeginMenu("Visualization")) {
                if (ImGui::MenuItem("Vector field view", "")) {
                    sim_params.display_option = SHOW_VECTOR_FIELD;
                }
                if (ImGui::MenuItem("Volume Render", "")) {
                    sim_params.display_option = VOL_DISPLAY;
                }
                if (ImGui::MenuItem("XY slice", "")) {
                    sim_params.display_option = SLICE_XY;
                }
                if (ImGui::MenuItem("ZY slice", "")) {
                    sim_params.display_option = SLICE_ZY;
                }
                if (ImGui::MenuItem("ZY slice", "")) {
                    sim_params.display_option = SLICE_XZ;
                }
                ImGui::EndMenu();
            }
            // ImGui::EndMenuBar();
            ImGui::Text("fps: %f\n", (double)sim_params.fps);
            ImGui::Text("%s\n", (char *)text);
            // kinetic_prop_drawer.set_float_uniform("m", sim_params.m);
            // kinetic_prop_drawer.set_float_uniform("dt", sim_params.dt);
            // spatial_prop_drawer.set_float_uniform("dt", sim_params.dt/2.0);
            ImGui::End();
        }
        ImGui::Render();
        // ImGui_
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
        // #ifndef __EMSCRIPTEN__
        clock_gettime(CLOCK_MONOTONIC_RAW, &frame_times[1]);
        // if (k % 2 == 0 && k != 0) {
            double delta_t = time_difference_in_ms(&frame_times[0], &frame_times[1]);
            sim_params.fps = 1000.0/delta_t;
	    // std::cout << psi2.sum_reduction().as_dvec4.x << std::endl;
            /*std::cout << "frames/s: " << (1000.0/delta_t) << std::endl;
            std::cout << "steps/s: " << (1000.0/delta_t)*sim_params.steps_per_frame
                      << std::endl;*/
        // }
        std::swap(frame_times[1], frame_times[0]);
        // #endif
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop)
        loop();
    #endif

    return exit_status;

}