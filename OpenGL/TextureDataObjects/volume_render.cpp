#include "volume_render.hpp"
#include "quaternions.hpp"
#include <GLES3/gl3.h>

#include <vector>

typedef unsigned int element_type;


static std::vector<Vec2> new_vertices(size_t &sizeof_vertices, 
                                      IVec2 render_texel_dimensions_2d,
                                      IVec3 render_texel_dimensions_3d) {
    size_t s = 6;
    size_t number_of_vertices = s*render_texel_dimensions_3d.length;
    sizeof_vertices = number_of_vertices*sizeof(Vec2);
    auto vertices = std::vector<Vec2> (number_of_vertices);
    for (int i = 0; i < render_texel_dimensions_3d.length; i++) {
        /* Note that each of the slices need to be constructed from
        back to front since this is the direction of blending.
        */
        int j = render_texel_dimensions_3d.length - i - 1;
        int width_3d = render_texel_dimensions_3d.width;
        int width_2d = render_texel_dimensions_2d.width;
        double w_ratio = double(width_3d)/double(width_2d);
        double edge = double(width_3d-1)/double(width_2d);
        // fprintf(stdout, "%g\n", i*w_ratio);
        vertices[s*j].x = i*w_ratio;
        vertices[s*j].y = 0.0;
        vertices[s*j+1].x = i*w_ratio + edge;
        vertices[s*j+1].y = 0.0;
        vertices[s*j+2].x = i*w_ratio + edge;
        vertices[s*j+2].y = 1.0;
        
        vertices[s*j+3].x = i*w_ratio + edge;
        vertices[s*j+3].y = 1.0;
        vertices[s*j+4].x = i*w_ratio;
        vertices[s*j+4].y = 1.0;
        vertices[s*j+5].x = i*w_ratio;
        vertices[s*j+5].y = 0.0;
    }
    return vertices;
}

/* Initialize the programs. */
void VolumeRender::init_programs() {
    // Program for sampling from the texture
    // that stores the 3D data
    this->programs.sample_volume = make_quad_program(
        "./shaders/vol-render/sample.frag"
    );
    // Program for showing the volume data
    this->programs.show_volume = make_program(
        "./shaders/vol-render/display.vert", 
        "./shaders/vol-render/display.frag");

    this->programs.gradient = make_quad_program(
        "./shaders/gradient/gradient3d.frag"
    );
    this->programs.cube = make_program(
        "./shaders/util/shape.vert",
        "./shaders/util/colour.frag");
}

/* Initialize all of the frames. */
void VolumeRender::init_frames() {

    // Create RGBA32F quads with the same dimensions as the sample
    struct TextureParams tex_params_sample_32f {
        .format=GL_RGBA32F, 
        .width=this->sample_texel_dimensions_2d.width,
        .height=this->sample_texel_dimensions_2d.height,
        .generate_mipmap=1,
        // .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .wrap_s=GL_REPEAT, .wrap_t=GL_REPEAT,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR,
    };
    glViewport(0, 0, 
               this->sample_texel_dimensions_2d.width, 
               this->sample_texel_dimensions_2d.height);
    // this->frames.draw = new_quad(&tex_params_sample_32f);
    this->frames.gradient = new_quad(&tex_params_sample_32f);
    // this->frames.boundary_mask = new_quad(&tex_params_sample_32f);

    // Create RGBA16F quads with the same dimensions as the sample
    struct TextureParams tex_params_sample_16f {
        .format=GL_RGBA16F, 
        .width=this->sample_texel_dimensions_2d.width,
        .height=this->sample_texel_dimensions_2d.height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR,
    };
    glViewport(0, 0, 
               this->sample_texel_dimensions_2d.width, 
               this->sample_texel_dimensions_2d.height);
    this->frames.vol_half_precision = new_quad(&tex_params_sample_16f);
    this->frames.gradient_half_precision = new_quad(&tex_params_sample_16f);

    // Create RGBA16F quads with the
    // same dimensions as the volume render texture
    struct TextureParams tex_params_render_16f {
        .format=GL_RGBA16F, 
        .width=this->render_texel_dimensions_2d.width,
        .height=this->render_texel_dimensions_2d.height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR,
    };
    glViewport(0, 0, 
               this->render_texel_dimensions_2d.width, 
               this->render_texel_dimensions_2d.height);
    // this->frames.sub_view3 = new_quad(&tex_params_render_16f);
    // this->frames.sub_view4 = new_quad(&tex_params_render_16f);
    this->frames.sample_volume
         = new_quad(&tex_params_render_16f);
    this->frames.sample_grad
         = new_quad(&tex_params_render_16f);

    // Begin construction of non quad frames.

    // Construct Vertices
    size_t sizeof_vertices {};
    auto vertices = new_vertices(sizeof_vertices, 
                                 this->render_texel_dimensions_2d,
                                 this->render_texel_dimensions_3d);
    
    // Construct elements
    /*auto elements = new_elements(&this->sizeof_elements,
                                 this->render_texel_dimensions_2d, 
                                 this->render_texel_dimensions_3d);*/

    struct TextureParams tex_params_view_16f_mipmap_filter {
        .format=GL_RGBA16F, 
        .width=this->view_dimensions.width,
        .height=this->view_dimensions.height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        // #ifdef __EMSCRIPTEN__
        .min_filter=GL_LINEAR,
        .mag_filter=GL_LINEAR,
        // #else
        // .min_filter=GL_LINEAR_MIPMAP_LINEAR, 
        // .mag_filter=GL_LINEAR_MIPMAP_LINEAR,
        // #endif
    };
    /*this->frames.out = new_frame(&tex_params_view_16f_mipmap_filter, 
                                 (float *)&vertices[0],
                                 sizeof_vertices,
                                 (element_type *)&elements[0],
                                 this->sizeof_elements);*/
    // fprintf(stdout, "%d\n", (int)sizeof_vertices);
    this->frames.out = new_frame(&tex_params_view_16f_mipmap_filter, 
                                 (float *)&vertices[0],
                                 (int)sizeof_vertices,
                                 NULL, 0);

}

VolumeRender::VolumeRender(IVec2 view_dimensions,
                           IVec3 render_dimensions, 
                           IVec3 sample_dimensions) {
    this->debug_rotation = Quaternion {{{0.0, 0.0, 0.0, 1.0}}};
    for (int i = 0; i < 3; i++) {
        if (i < 2)
            this->view_dimensions.ind[i] = view_dimensions.ind[i];
        this->sample_texel_dimensions_3d.ind[i] = sample_dimensions.ind[i];
        this->render_texel_dimensions_3d.ind[i] = render_dimensions.ind[i];
    }
    this->sample_texel_dimensions_2d.width
        = sample_dimensions.width*sample_dimensions.length;
    this->sample_texel_dimensions_2d.height = sample_dimensions.height;
    this->render_texel_dimensions_2d.width
         = render_dimensions.width*render_dimensions.length;
    this->render_texel_dimensions_2d.height = render_dimensions.height;

    init_programs();
    init_frames();
    
}

static void gradient(frame_id dst,
                     frame_id volume_data,
                     frame_id boundary_mask,
                     int gradient_program,
                     int order_of_accuracy, 
                     int boundary_type,
                     int staggered_mode, 
                     int index,
                     const struct IVec3 &texel_dimensions_3d,
                     const struct IVec2 &texel_dimensions_2d) {
    bind_quad(dst, gradient_program);
    set_sampler2D_uniform("tex", volume_data);
    set_int_uniform("orderOfAccuracy", order_of_accuracy);
    set_int_uniform("boundaryType", boundary_type);
    // set_sampler2D_uniform("boundaryMaskTex", this->frames.boundary_mask);
    set_int_uniform("staggeredMode", staggered_mode);
    set_int_uniform("index", index);
    set_ivec2_uniform("texelDimensions2D", 
                      texel_dimensions_2d.width, 
                      texel_dimensions_2d.height);
    set_ivec3_uniform("texelDimensions3D", 
                      texel_dimensions_3d.width,
                      texel_dimensions_3d.height,
                      texel_dimensions_3d.length);
    set_vec3_uniform("dr", 1.0, 1.0, 1.0);
    set_vec3_uniform("dimensions3D",
                     (float)texel_dimensions_3d.width,
                     (float)texel_dimensions_3d.height,
                     (float)texel_dimensions_3d.length);
    draw_unbind_quad();

}


/* Function for the program that uses the "sample-vol3d.frag" shader.
The volume_tex quad stores the initial raw 3D volumetric data as a
2D texture, while the dst quad contains the texture used for the 
volume rendering frame. This function is used to transform the texture
data contained in volume_tex to its corresponding representation in the
dst quad.  

This corresponds to the sampling step given in the Wikipedia page
for volume ray casting.

References:

Volume ray casting - Wikipedia
https://en.wikipedia.org/wiki/Volume_ray_casting

*/
static void sample_volume_data(frame_id dst, frame_id volume_tex, 
                               int sample_volume_program,
                               double view_scale,
                               Quaternion &rotation,
                               const struct IVec3 &render_texel_dimensions_3d,
                               const struct IVec3 &sample_texel_dimensions_3d,
                               const struct IVec2 &sample_texel_dimensions_2d
                               ) {
    bind_quad(dst, sample_volume_program);
    set_sampler2D_uniform("tex", volume_tex);
    set_float_uniform("viewScale", view_scale);
    set_vec4_uniform("rotation", 
                    rotation.x, rotation.y, 
                    rotation.z, rotation.w);
    set_ivec3_uniform("renderTexelDimensions3D",
                      render_texel_dimensions_3d.width,
                      render_texel_dimensions_3d.height,
                      render_texel_dimensions_3d.length);
    set_ivec3_uniform("sampleTexelDimensions3D",
                      sample_texel_dimensions_3d.width,
                      sample_texel_dimensions_3d.height,
                      sample_texel_dimensions_3d.length);
    set_ivec2_uniform("sampleTexelDimensions2D",
                      sample_texel_dimensions_2d.width,
                      sample_texel_dimensions_2d.height);
    draw_unbind_quad();
}

/*This takes as input the sample_vol quad frame that contains the 
volume data interpolated to those points along the volume render frame,
as well as its corresponding gradient data which is contained in the 
sample_grad quad frame, which is used to construct the final volume
render which is stored in the dst frame.

This corresponds to the shading step as given on the Wikipedia
page for Volume ray casting.

References:

Volume ray casting - Wikipedia
https://en.wikipedia.org/wiki/Volume_ray_casting
*/
static void show_sampled_vol(frame_id dst, 
                             frame_id sample_grad, frame_id sample_vol,
                             int show_vol_program,
                             int cube_program,
                             int sizeof_elements,
                             const Quaternion &rotation,
                             const Quaternion &debug_rotation,
                             double scale,
                             const IVec3 &render_texel_dimensions_3d,
                             const IVec2 &render_texel_dimensions_2d) {
    bind_frame(dst, show_vol_program);
    struct VertexParam vertex_params[2] = {
        {.name=(char *)"uvIndex", .size=2, .type=GL_FLOAT, 
        .normalized=GL_FALSE,
        .stride=0, .offset=0}
    };
    set_vertex_attributes(vertex_params, 1);
    // Set vertex uniforms
    set_vec4_uniform("debugRotation",
                     debug_rotation.x,
                     debug_rotation.y,
                     debug_rotation.z,
                     debug_rotation.w);
    set_float_uniform("scale", scale);
    set_ivec3_uniform("texelDimensions3D",
                      render_texel_dimensions_3d.width,
                      render_texel_dimensions_3d.height,
                      render_texel_dimensions_3d.length);
    set_ivec3_uniform("fragmentTexelDimensions3D",
                      render_texel_dimensions_3d.width,
                      render_texel_dimensions_3d.height,
                      render_texel_dimensions_3d.length);
    /* set_ivec2_uniform("texelDimensions2D",
                      render_texel_dimensions_2d.width,
                      render_texel_dimensions_2d.height);*/
    set_ivec2_uniform("fragmentTexelDimensions2D",
                      render_texel_dimensions_2d.width,
                      render_texel_dimensions_2d.height);
    // Set fragment uniforms
    // set_int_uniform("debugShow2DTexture", 0);
    set_vec4_uniform("rotation", rotation.x, rotation.y,
                     rotation.z, rotation.w);
    set_sampler2D_uniform("gradientTex", sample_grad);
    set_sampler2D_uniform("densityTex", sample_vol);
    glDrawArrays(GL_TRIANGLES, 0, 
                 6*render_texel_dimensions_3d.length);
    unbind();

    // glDisable(GL_BLEND);
    /* bind_frame(dst, cube_program);
    set_vertex_attributes(vertex_params, 1);
    set_vec4_uniform("rotation", rotation.x, rotation.y,
                     rotation.z, rotation.w);
    set_float_uniform("scale", scale);
    glDrawArrays(GL_TRIANGLES, 0, 
                 6*render_texel_dimensions_3d.length);
    unbind();*/
}

Texture2DData VolumeRender::render(const Texture2DData &volume_data,
                                   float view_scale, Quaternion rotation
                                   ) const {

    glViewport(
        0, 0, 
        sample_texel_dimensions_2d.width,
        sample_texel_dimensions_2d.height);

    // Half precision volume data.
    tex_copy(this->frames.vol_half_precision, volume_data.get_frame_id());


    // Take the gradient of the volume data.
    gradient(this->frames.gradient_half_precision, volume_data.get_frame_id(),
             0, this->programs.gradient, 
             2, 0, 0, 3, 
             this->sample_texel_dimensions_3d, 
             this->sample_texel_dimensions_2d);
    
    // Half precision gradient data
    // tex_copy(this->frames.gradient_half_precision, this->frames.gradient);
    
    glViewport(
        0, 0, 
        render_texel_dimensions_2d.width,
        render_texel_dimensions_2d.height);

    // Sample the volume data so that it could be written to 
    // the render texture.
    sample_volume_data(this->frames.sample_volume,
                       // volume_data.get_frame_id(),
                       this->frames.vol_half_precision, 
                       this->programs.sample_volume, 
                       view_scale, 
                       rotation,
                       this->render_texel_dimensions_3d,
                       this->sample_texel_dimensions_3d,
                       this->sample_texel_dimensions_2d);
    // Sample the gradient data to its own render texture.
    sample_volume_data(this->frames.sample_grad,
                       // this->frames.gradient,
                       this->frames.gradient_half_precision, 
                       this->programs.sample_volume,
                       view_scale, 
                       rotation,
                       this->render_texel_dimensions_3d,
                       this->sample_texel_dimensions_3d,
                       this->sample_texel_dimensions_2d);

    glViewport(0, 0, view_dimensions.width, view_dimensions.height);

    /* Blend each of the translucent slices of the volume render frame together.
    This is part of the final composting step for constructing the volume render,
    as outlined in the Wikipedia page on volume ray casting.

    Wikipedia - Volume ray casting
    https://en.wikipedia.org/wiki/Volume_ray_casting
    
    Blending reference:

    Vries, J. Blending. In Learn OpenGL.
    https://learnopengl.com/Advanced-OpenGL/Blending
    
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    /* GLboolean depth_test_prev_enabled = glIsEnabled(GL_DEPTH_TEST);
    if (!depth_test_prev_enabled)
        glEnable(GL_DEPTH_TEST);*/
    {

        show_sampled_vol(this->frames.out,
                        this->frames.sample_grad, 
                        this->frames.sample_volume,
                        this->programs.show_volume, 
                        this->programs.cube,
                        this->sizeof_elements,
                        rotation,
                        this->debug_rotation,
                        view_scale,
                        render_texel_dimensions_3d, 
                        render_texel_dimensions_2d);

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClear(GL_STENCIL_BUFFER_BIT);

    }
    /*if (!depth_test_prev_enabled)
        glDisable(GL_DEPTH_TEST);*/

    glDisable(GL_BLEND);


    Texture2DData t = Texture2DData(
        HALF_FLOAT4, 
        this->view_dimensions.width, 
        this->view_dimensions.height,
        false, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_LINEAR, GL_LINEAR);
    tex_copy(t.get_frame_id(), this->frames.out);
    return t;
}

Texture2DData VolumeRender::debug_get_vol_half_precision() const {
    glViewport(0, 0, view_dimensions.width, view_dimensions.height);
    Texture2DData t = Texture2DData(
        FLOAT4, 
        this->view_dimensions.width, 
        this->view_dimensions.height,
        true, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_LINEAR, GL_LINEAR);
    tex_copy(t.get_frame_id(), this->frames.vol_half_precision);
    return t;
}

Texture2DData VolumeRender::debug_get_grad_half_precision() const {
    glViewport(0, 0, view_dimensions.width, view_dimensions.height);
    Texture2DData t = Texture2DData(
        FLOAT4, 
        this->view_dimensions.width, 
        this->view_dimensions.height,
        true, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_LINEAR, GL_LINEAR);
    tex_copy(t.get_frame_id(), this->frames.gradient_half_precision);
    return t;
}

Texture2DData VolumeRender::debug_get_sample_grad() const {
    glViewport(0, 0, view_dimensions.width, view_dimensions.height);
    Texture2DData t = Texture2DData(
        FLOAT4, 
        this->view_dimensions.width, 
        this->view_dimensions.height,
        true, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_LINEAR, GL_LINEAR);
    tex_copy(t.get_frame_id(), this->frames.sample_grad);
    return t;
}

Texture2DData VolumeRender::debug_get_sample_vol() const {
    glViewport(0, 0, view_dimensions.width, view_dimensions.height);
    Texture2DData t = Texture2DData(
        FLOAT4, 
        this->view_dimensions.width, 
        this->view_dimensions.height,
        true, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_LINEAR, GL_LINEAR);
    tex_copy(t.get_frame_id(), this->frames.sample_volume);
    return t;
}