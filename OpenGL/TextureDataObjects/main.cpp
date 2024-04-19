// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
#include "texture_data.hpp"
#include "draw_texture_data.hpp"
// #include <OpenGL/OpenGL.h>
#include <vector>
#include "fft.h"
#include <iostream>
#include <ctime>
#include "bitmap.h"
#include "summation.h"
#include <GLES3/gl3.h>

#include "texture_data_transfers_stuff.hpp"
#include "complex_func.hpp"
#include "summation_stuff.hpp"
#include "reduction_to_1d.hpp"
#include "poisson.hpp"
#include "waves.hpp"
#include "reaction_diffusion.hpp"
#include "stable_fluids.hpp"
#include "n_body.hpp"
#include "mol_dynamics.hpp"
#include "schrod_leapfrog.hpp"
#include "schrod_splitstep.hpp"
#include "schrod_splitstep_3d.hpp"
#include "schrod_leapfrog_3d.hpp"
#include "schrod_splitstep_image_potential.hpp"
#include "dirac_leapfrog.hpp"
#include "dirac_splitstep_3d.hpp"
#include "isf_splitstep.hpp"
#include "sph.hpp"
#include "electrodynamics_3d.hpp"
#include "pauli_leapfrog_3d.hpp"
#include "free_space_position.hpp"
#include "render.hpp"


int main(int argc, char **argv) {
    int WIDTH_TO_HEIGHT = 1;
    #ifdef __APPLE__
    // int NX = 512, NY = 512;
    // int NX = 1024, NY = 1024;
    int NX = WIDTH_TO_HEIGHT*1440, NY = 1440;
    #else
    int NX = WIDTH_TO_HEIGHT*640, NY = 640;
    #endif
    GLFWwindow *window = init_window(NX, NY);
    /* int monitor_count = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);
    GLFWmonitor *primary = glfwGetPrimaryMonitor();
    int modes_count = 0;
    // GLFWvideomode *modes = glfwGetVideoModes(primary, &monitor_count);
    int w, h;
    glfwGetMonitorPhysicalSize(primary, &w, &h);
    printf("%d, %d\n", w, h);*/
    init_bin_ops_programs();
    init_unary_ops_programs();
    init_fft_programs();
    init_summation_programs();
    frame_id main_frame = new_quad(NULL);
    std::vector<int (*)(Renderer *renderer)> functions {
        reduction_to_1d, // Doesn't work
        summation_stuff,
        waves,
        particles_coulomb, // Slow
        stable_fluids,
        isf_splitstep,
        // For the above there is extreme lag and doesn't work for older systems
        sph_mt, // Slow
        particles_lennard_jones, // Slow
        particles_lennard_jones_mt,
        // The above works but is really slow and requires sending headers
        // through the server (see run.py). Also got a warning about blocking
        // in the main thread.
        schrod_leapfrog,
        schrod_leapfrog_3d,
        pauli_leapfrog_3d,
        schrod_splitstep,
        schrod_splitstep_3d,
        dirac_leapfrog, // Slow and doesn't work properly for  older systems
        dirac_splitstep_3d,
        /* #ifndef __EMSCRIPTEN__
        schrod_splitstep_image_potential,
        #endif
        // Above will not work since it requires reading a png file,
        // and the png library appears to not be available for emscripten.*/
        gray_scott_reaction_diffusion,
        electrodynamics_3d,
        free_space_position,
    };
    int which_function = 15;
    if (argc > 1)
        try {
            which_function = std::stoi(argv[1]) % functions.size();
        } catch (std::exception &e) {
    }
    struct Renderer renderer;
    renderer.window = window;
    renderer.main_frame = main_frame;
    for (int n = which_function; n < functions.size(); n++) {
        int exit_status = functions[n](&renderer);
        // int exit_status = functions[n](window, main_frame);
        delete_all_frames();
        printf("\n");
        if (exit_status != 0) break;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
