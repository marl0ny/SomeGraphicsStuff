#include "simulation.hpp"
#include "tex_sort.hpp"

using namespace sim_2d;


static struct IVec2 decompose(unsigned int n) {
    struct IVec2 d = {.ind={(int)n, 1}};
    int i = 1;
    for (; i*i < n; i++) {}
    for (; n % i; i--) {}
    d.ind[0] = ((n / i) > i)? (n / i): i;
    d.ind[1] = ((n / i) < i)? (n / i): i;
    return d;
}

static const std::vector<float> QUAD_VERTICES = {
    -1.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0, 0.0};
static const std::vector<int> QUAD_ELEMENTS = {0, 1, 2, 0, 2, 3};
static WireFrame get_quad_wire_frame() {
    return WireFrame(
        {{"position", Attribute{
            3, GL_FLOAT, false,
            0, 0}}},
        QUAD_VERTICES, QUAD_ELEMENTS,
        WireFrame::TRIANGLES
    );
}

Programs::Programs() {
    this->copy = Quad::make_program_from_path(
        "./shaders/util/copy.frag"
    );
    this->sort4 = Quad::make_program_from_path(
        "./shaders/sort/sort4.frag"
    );
    this->user_defined = 0;
}

#define N 32768

Frames::
Frames(const TextureParams &default_tex_params, const SimParams &params):
    default_tex_params(default_tex_params),
    sim_tex_params({
        .format=GL_RGBA32F,
        .width=(unsigned int)decompose(N)[0],
        .height=(unsigned int)decompose(N)[1],
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=GL_NEAREST,
        .mag_filter=GL_NEAREST,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    data(sim_tex_params),
    tmp(sim_tex_params),
    render_tmp(default_tex_params),
    render(default_tex_params),
    quad_wire_frame(get_quad_wire_frame()) {
}

static std::vector<Vec4> random_vec_vec4(
    int n_samples, bool is_bitonic=false) {
    std::vector<Vec4> arr(
        n_samples, {.ind{0.0, 0.0, 0.0, 0.0}});
    float max_val = 0.0;
    for (int i = 0; i < n_samples; i++) {
        srand(time(NULL) + i + rand());
        int val = rand() % N;
        if (is_bitonic) {
            for (int k = 0; k < 3; k++) {
                if (i == 0)
                    arr[i][k] = val/float(N);
                else if (i > 0 && i < n_samples/2)
                    arr[i][k] = arr[i-1][k] + val/float(N);
                else if (i >= n_samples/2)
                    arr[i][k] = arr[i-1][k] - val/float(N);
            }
            max_val = (arr[i][0] > max_val)? arr[i][0]: max_val;
        } else {
            for (int k = 0; k < 3; k++)
                arr[i][k] = val/float(N);
        }
        arr[i][3] = 1.0;
    }
    if (is_bitonic)
        for (int i = 0; i < n_samples; i++)
            for (int k = 0; k < 3; k++)
                arr[i][k] *= (1.0/max_val);
    return arr;
}

Simulation::
Simulation(const TextureParams &default_tex_params, const SimParams &params
) : m_programs(), m_frames(default_tex_params, params), 
    m_sort(decompose(N)) {
    int n_samples 
        = m_frames.sim_tex_params.width*m_frames.sim_tex_params.height;
    std::vector<Vec4> arr = random_vec_vec4(n_samples);
    m_frames.data.set_pixels((float *)&arr[0]);
    m_sort(m_frames.tmp, m_frames.data);
    // m_frames.tmp.draw(
    //     m_programs.sort4,
    //     {
    //         {"tex", &m_frames.data},
    //         {"texDimensions2D", IVec2{
    //                 .x=(int)m_frames.sim_tex_params.width,
    //                 .y=(int)m_frames.sim_tex_params.height}}
    //         // {"isAscending", int(true)}
    //     }
    // );

    std::vector<float> pixels = m_frames.tmp.get_float_pixels();
    Vec4 *pixels_ptr = (Vec4 *)&pixels[0];
    for (int i = 0; i < n_samples; i++) {
        if (i % 4 == 0 && i != 0)
            puts("");
        printf("%d: %g, %g\n", i, arr[i][0], pixels_ptr[i][0]);
    }
    printf("\n");
    // for (int i = 0; i < n_samples/4; i++) {
    //     if (i % 2)
    //         printf("High to low: ");
    //     else
    //         printf("Low to high: ");
    //     printf("%g, %g, %g, %g\n",
    //         pixels_ptr[4*i].r,
    //         pixels_ptr[4*i+1].r,
    //         pixels_ptr[4*i+2].r, 
    //         pixels_ptr[4*i+3].r);
    // }
    printf("%d, %d\n", 
        m_frames.sim_tex_params.width, m_frames.sim_tex_params.height);
}

const RenderTarget &Simulation::view(SimParams &params) {
    m_frames.data.draw(
        m_programs.copy,
        {
            {"tex", &m_frames.tmp}
        }
    );
    m_frames.render.draw(
        m_programs.copy,
        {{"tex", {&m_frames.data}}},
        m_frames.quad_wire_frame
    );
    return m_frames.render;
}