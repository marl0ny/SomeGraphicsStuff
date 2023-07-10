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
#include "schrod_splitstep_image_potential.hpp"
#include "dirac_leapfrog.hpp"
#include "isf_splitstep.hpp"
#include "sph.hpp"

int main() {
    int WIDTH_TO_HEIGHT = 1;
    #ifdef __APPLE__
    // int NX = 512, NY = 512;
    // int NX = 1024, NY = 1024;
    int NX = WIDTH_TO_HEIGHT*1440, NY = 1440;
    #else
    int NX = WIDTH_TO_HEIGHT*512, NY = 512;
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
    std::vector<int (*)(GLFWwindow *, frame_id)> functions {
        // reduction_to_1d,
        // summation_stuff,
        // waves,
        // particles_coulomb,
        // stable_fluids,
        // isf_splitstep,
        sph_mt,
        // particles_lennard_jones,
        // particles_lennard_jones_mt,
        // schrod_leapfrog,
        // schrod_splitstep,
        // dirac_leapfrog,
        // schrod_splitstep_image_potential,
        // gray_scott_reaction_diffusion
    };
    for (auto &f: functions) {
        int exit_status = f(window, main_frame);
        delete_all_frames();
        printf("\n");
        if (exit_status != 0) break;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
