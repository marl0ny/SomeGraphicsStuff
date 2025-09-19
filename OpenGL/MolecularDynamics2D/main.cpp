#include "gl_wrappers.hpp"
#include "glfw_window.hpp"
#include "parameters.hpp"
#include "interactor.hpp"
#include "simulation.hpp"

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif
#include <functional>
#include <utility>

#include "wasm_wrappers.hpp"


static std::function <void()> s_loop;
#ifdef __EMSCRIPTEN__
static void s_main_loop() {
    s_loop();
}
#endif

using namespace sim_2d;


static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

static std::string double_as_string(double val) {
    double log_val = std::log(std::abs(val)), log_10 = std::log(10.0);
    double power = std::floor(log_val/log_10);
    double base = val/std::pow(10.0, power);
    if (std::abs(power) <= 3.0)
        return std::to_string(val);
    return std::to_string(base) + "e" + std::to_string(int(power));
}

static std::string double_as_string(double val, unsigned int decimal_places) {
    if (std::isnan(val) || std::isinf(val))
        return std::to_string(val);
    double log_val = std::log(std::abs(val)), log_10 = std::log(10.0);
    double power = std::floor(log_val/log_10);
    double base = val/std::pow(10.0, power);
    if (std::abs(power) <= 3.0) {
        // TODO: Apply significant digits to this as well!
        return std::to_string(val);
    }
    base = std::round(
        base*std::pow(10.0, decimal_places))
            / std::pow(10.0, decimal_places);
    std::string base_string = std::to_string(base);
    int k = base_string.length() - 1;
    for (; base_string[k] == '0' && k > 0; k--);
    return base_string.substr(0, k+1) + "e" + std::to_string(int(power));
}

void simulation_ui_interface_handler(
    MainGLFWQuad main_render,
    TextureParams default_tex_params,  // Default texture parameters
    SimParams &params  // Parameters of the simulations
) {
    Interactor interactor(main_render.get_window());
    Simulation sim(default_tex_params, params);
    SimParams modified_params {};

    // For handling mouse or touch interation.
    std::optional<Vec2> start_position;
    std::vector<Vec2> cursor_positions {};
    std::optional<std::pair<Vec2, Vec2>> start_double_touches;
    std::vector<std::pair<Vec2, Vec2>> double_touches_positions {};
    Quaternion rotation = Quaternion{.i=0.0, .j=0.0, .k=0.0, .real=1.0};

    Vec2 translation = Vec2{.x=0.0, .y=0.0};

    {
        /* Set those parameters of the Parameters struct that are treated
        as uniforms by GLSL shaders.*/
        s_sim_params_set = [&params]
            (int c, Uniform u) {
            params.set(c, u);
        };
        /* Get those parameters of the Parameters struct that can be
        inputed as uniforms to GLSL shaders.*/
        s_sim_params_get = [&params]
            (int c) -> Uniform {
            return params.get(c);
        };
        /* String parametres can't be configured as uniforms, so
        are set using a different function.*/
        s_sim_params_set_string = [&params]
            (int c, int index, std::string val) {
            params.set(c, index, val);
        };
        /* Perform an action upon the press of a button. */
        s_button_pressed = []
            (int param_code) {
        };
        /* Upon a change of a dropdown or selection menu, change its
        corresponding selection parameter in the Parameters struct so that
        it matches the dropdown.*/
        s_selection_set = [&params, &sim]
            (int c, int val) {
            if (c == params.PARTICLE_COUNT_SELECTOR) {
                params.particleCountSelector.selected = val;
                params.particleCount = (int)std::stol(
                params.particleCountSelector.options[val]);
                sim.reset_number_of_particles(params);
                params.nSteps = 0;
            }
            if (c == 
                params.NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION_SELECTOR) {
                params.neighbourCellCountForForceComputationSelector.selected
                    = val;
                std::string str_count
                    = params
                        .neighbourCellCountForForceComputationSelector
                            .options[val];
                params.neighbourCellCountForForceComputation =
                    (int)std::stol(str_count);
            }
        };
    }

    struct timespec frame_time[2];
    clock_gettime(CLOCK_MONOTONIC, &frame_time[0]);
    double dt = params.requestedDt;

    s_loop = [&] {

        for (int i = 0; i < params.stepsPerFrame; i++) {
            dt = sim.time_step(params);
            params.nSteps += 1;
        }
        float energy;
        printf("%g\n", energy = sim.get_energy());
        std::string text_content2 = "Total energy (ought to be conserved) = " 
            + double_as_string(energy, 3);
        edit_label_display(params.TOTAL_E_LABEL, text_content2);
        printf("%g\n", dt);
        std::cout << dt/pow(10.0, std::floor(std::log(dt)/std::log(10.0))) << std::endl;
        std::cout << std::to_string((double)dt) << std::endl;
        std::string text_content3 = "Actual time step = " 
                + double_as_string(dt, 3);
        edit_label_display(params.DT_LABEL, text_content3);
        std::string text_content4 = 
            "Cummulative total energy drift "\
                "error (percentage of initial energy) = "
                + double_as_string(
                    sim.get_energy_drift_error_percentage(), 3);
        edit_label_display(params.DRIFT_E_LABEL, text_content4);

        if (start_position.has_value()) {
            if (cursor_positions.size() > 1) {
                Vec2 delta_2d = interactor.get_mouse_delta();
                Vec3 delta {.ind={delta_2d[0], delta_2d[1], 0.0}};
                translation 
                    = translation 
                        - 2.0*Vec2{.x=delta.x,.y=delta.y}
                            /(0.05*Interactor::get_scroll());
                Vec3 view_vec {.ind={0.0, 0.0, -1.0}};
                Vec3 axis = cross_product(delta, view_vec);
                Quaternion rot = Quaternion::rotator(
                    3.0*axis.length(), axis);
                rotation = rotation*rot;
            }
        }
        main_render.draw(
            sim.view(params, 0.05*Interactor::get_scroll(), translation));

        auto poll_events = [&] {
            // Tell GLFW to poll events
            glfwPollEvents();

            // Get user interaction events
            interactor.click_update(main_render.get_window());

            // Handle mouse or single touch events
            Vec2 pos = interactor.get_mouse_position();
            if (pos.x > 0.0 && pos.x < 1.0 && 
                pos.y > 0.0 && pos.y < 1.0 && 
                interactor.left_pressed()) {
                if (!start_position.has_value())
                    start_position = pos;
                cursor_positions.push_back(pos);
            }
            if (interactor.left_released()) {
                if (start_position.has_value()) {
                    start_position.reset();
                    cursor_positions.clear();
                }
            }

            // Handle double touch events
            Vec2 double_touches[2];
            double_touches[0] = interactor.get_double_touch_position(0);
            double_touches[1] = interactor.get_double_touch_position(1);
            if (interactor.double_touch_active()
                && double_touches[0].x > 0.0 && double_touches[0].x < 1.0
                && double_touches[0].y > 0.0 && double_touches[0].y < 1.0
                && double_touches[1].x > 0.0 && double_touches[1].x < 1.0
                && double_touches[1].y > 0.0 && double_touches[1].y < 1.0) {
                if (!start_double_touches.has_value())
                    start_double_touches = 
                        {double_touches[0], double_touches[1]};
                double_touches_positions.push_back(
                        {double_touches[0], double_touches[1]});
            }
            if (interactor.double_touch_released()) {
                if (start_double_touches.has_value()) {
                    start_double_touches.reset();
                    double_touches_positions.clear();
                }
            }

            #ifndef __EMSCRIPTEN__
            #endif
        };
        poll_events();

        glfwSwapBuffers(main_render.get_window());

        if ((params.nFrames % 5 == 0) && params.nFrames > 0) {
            clock_gettime(CLOCK_MONOTONIC, &frame_time[1]);
            double delta_t = time_difference_in_ms(
                &frame_time[0], &frame_time[1])/5.0;
            // printf("Frames/s = %g\n", floor((1000.0/delta_t)));
            std::string text_content = "Frames/s = " 
                + std::to_string(int(floor((1000.0/delta_t))));
            edit_label_display(params.FPS_LABEL, text_content);
            std::swap(frame_time[0], frame_time[1]);
        }
        params.nFrames += 1;
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(s_main_loop, 0, true);
    #else
    while (!glfwWindowShouldClose(main_render.get_window()))
        s_loop();
    #endif
}


int main(int argc, char *argv[]) {
    int window_width = 1440, window_height = 1440;
    if (argc >= 3) {
        window_width = std::atoi(argv[1]);
        window_height = std::atoi(argv[2]);
    }
    int filter_type = GL_LINEAR;
    if (argc >= 4) {
        std::string s(argv[3]);
        if (s == "nearest")
            filter_type = GL_NEAREST;
    }
    SimParams params {};
    TextureParams default_tex_params = {
        .format=(unsigned int)
            ((filter_type == GL_NEAREST)? GL_RGBA8: GL_RGBA32F),
        .width=(unsigned int)window_width,
        .height=(unsigned int)window_height,
        .generate_mipmap=!(filter_type == GL_NEAREST),
        .wrap_s=GL_CLAMP_TO_EDGE,
        .wrap_t=GL_CLAMP_TO_EDGE,
        .mag_filter=(unsigned int)filter_type,
        .min_filter=(unsigned int)filter_type
    };
    MainGLFWQuad 
    main_render (default_tex_params.width, default_tex_params.height);
    simulation_ui_interface_handler(
        main_render, default_tex_params, params);
    return 0;
}
