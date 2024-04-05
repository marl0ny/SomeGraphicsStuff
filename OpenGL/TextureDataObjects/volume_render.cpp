#include "volume_render.hpp"
#include "quaternions.hpp"
#include <GLES3/gl3.h>


enum {Z_ORIENTATION=0, X_ORIENTATION=1, Y_ORIENTATION=2};


/* Given the index used for accessing the 3D array, get its 1D counterpart.
*/
static int to_1d_index(IVec3 index_3d,
                       IVec3 texel_dimensions_3d) {
    /* This assumes that the data is stacked in columns.
    */
    int i = index_3d.i, j = index_3d.j, k = index_3d.k;
    int width = texel_dimensions_3d.width;
    int height = texel_dimensions_3d.height;
    // int length = texel_dimensions_3d.length;
    return i*height + j + k*width*height;
}

/* Write the elements for a single face. This returns the total
 number of elements used for making this face.

 elements - pointer location for the elements of a single face
 k - the index of which face to write the elements to.
 orientation - the direction where the faces are stacked
 texel_dimensions_2d - dimensions of the 2D texture
 texel_dimensions_3d - dimensions of the 3D array of data for the texture
*/
static int single_face(int *elements, int k, int orientation,
                       IVec2 texel_dimensions_2d,
                       IVec3 texel_dimensions_3d) {
    int horizontal_iter, vertical_iter;
    switch(orientation) {
        case Z_ORIENTATION:
        horizontal_iter = texel_dimensions_3d.height - 1;
        vertical_iter = texel_dimensions_3d.width - 2;
        case Y_ORIENTATION:
        horizontal_iter = texel_dimensions_3d.length - 1;
        vertical_iter = texel_dimensions_3d.width - 2;
        case X_ORIENTATION:
        horizontal_iter = texel_dimensions_3d.width - 1;
        vertical_iter = texel_dimensions_3d.height - 2;
    }
    int inc = 1;
    int j = 0;
    int elem_index = 0;
    /* By default, each square is constructed on top of each
    other, using the final upper left vertex from the last square
    as the first bottom left vertex for the next square. Once i
    reaches vertical_iter, start doing the next column by incrementing
    j, where the squares are constructed from top to bottom. 

    */
    for (int i = 0; j < horizontal_iter; i += inc) {
        if (k == 0)
            fprintf(stdout, "%d, %d\n", i, j);
        IVec3 bottom_left = {}, bottom_right = {};
        IVec3 upper_left = {}, upper_right = {};
        switch(orientation) {
            case Z_ORIENTATION:
            bottom_left = {.x=i, .y=j, .z=k};
            bottom_right = {.x=i+1, .y=j, .z=k};
            upper_right = {.x=i+1, .y=j+1, .z=k};
            upper_left = {.x=i, .y=j+1, .z=k};
            break;
            case Y_ORIENTATION:
            bottom_left = {.x=i, .y=k, .z=j};
            bottom_right = {.x=i+1, .y=k, .z=j};
            upper_right = {.x=i+1, .y=k, .z=j+1};
            upper_left = {.x=i, .y=k, .z=j+1};
            break;
            case X_ORIENTATION:
            bottom_left = {.x=k, .y=i, .z=j};
            bottom_right = {.x=k, .y=i+1, .z=j};
            upper_right = {.x=k, .y=i+1, .z=j+1};
            upper_left = {.x=k, .y=i, .z=j+1};
            break;
        }
        elements[elem_index++] = to_1d_index(bottom_left,
                                             texel_dimensions_3d);
        elements[elem_index++] = to_1d_index(bottom_right,
                                             texel_dimensions_3d);
        elements[elem_index++] = to_1d_index(upper_right, 
                                             texel_dimensions_3d);
        elements[elem_index++] = to_1d_index(upper_right, 
                                             texel_dimensions_3d);
        elements[elem_index++] = to_1d_index(bottom_left,
                                             texel_dimensions_3d);
        elements[elem_index++] = to_1d_index(upper_left,
                                             texel_dimensions_3d);
        if (i == 0) {
            if (inc == -1) {
                inc = 0;
                j++;
            } else if (inc == 0) {
                inc = 1;
            }
        } else if (i == vertical_iter) {
            if (inc == 1) {
                inc = 0;
                j++;
            } else if (inc == 0) {
                inc = -1;
            }
        } 
    }
    return elem_index;
}

/* Create the array of element indices for the entire
volume. This function writes the total number of bytes
used for the elements array to the location given
in the argument ptr_sizeof_elements.
*/
static int *new_elements(int *ptr_sizeof_elements,
                         IVec2 render_texel_dimensions_2d,
                         IVec3 render_texel_dimensions_3d) {
    if (!ptr_sizeof_elements) {
        fprintf(stderr, "new_elements: invalid ptr_sizepf_elements.");
        return nullptr;
    }
    // The variables width_3d and height_3d denotes the dimensions
    // of each face in terms of the number of vertices,and length_3d
    // denotes the total number of faces, stacked along the
    // z direction.
    // Each element except for the first connects with the preceding element.
    // For each face there are 6*(width_3d-1)*(height_3d-1) total elements,
    // since a face with width_3d*height_3d number of vertices requires
    // (width_3d-1)*(height_3d-1) squares, where each square is constructed
    // from two triangles which each consists of three vertices.
    // There are length_3d total faces in total, where a single
    // infinitely thin triangle must connect each face. Thus there are
    // 6*(width_3d-1)*(height_3d-1)*length_3d + 3*length_3d total elements.
    int number_of_elements = 6*(render_texel_dimensions_3d.width - 1)
                                *(render_texel_dimensions_3d.height - 1)
                                *render_texel_dimensions_3d.length
                                + 3*render_texel_dimensions_3d.length;
    *ptr_sizeof_elements = sizeof(int)*number_of_elements;
    // fprintf(stdout, "Number of elements: %d\n", number_of_elements);
    int *elements = (int *)calloc(number_of_elements, sizeof(int));
    // int *elements = new int[number_of_elements];
    if (!elements) {
        fprintf(stderr, "new_elements: unable to allocate elements.");
        return nullptr;
    }
    // fprintf(stdout, "Elements address %x\n", elements);
    int elem_index = 0;
    int length = render_texel_dimensions_3d.length;
    /* Currently the faces are only built along the z direction,    
    where the backmost face is done first. */
    for (int k = length - 1;  k >= 0; k--) {
        // fprintf(stdout, "%d\n", elem_index);
        // std::cout << elem_index << std::endl;
        elem_index += single_face(elements + elem_index, k, 
                                  Z_ORIENTATION, 
                                  render_texel_dimensions_2d, 
                                  render_texel_dimensions_3d);
        int index = elements[elem_index - 1];
        // Construct the triangle that connects each face.
        elements[elem_index++] = index;
        elements[elem_index++] = index;
        IVec3 indices = {{{0, 0, (k-1) % length}}};
        elements[elem_index++] = to_1d_index(indices, 
                                             render_texel_dimensions_3d);
    }
    return elements;
}

/* Construct the array of vertices for the volume render frame.
These vertices are given in 2D texture coordinate form, which will
be transformed to the 3D coordinates of the volume frame in the shaders.
The total number of bytes used for constructing the array is written
to the location given in the ptr_sizeof_vertices argument.
*/
static Vec4 *new_vertices(int *ptr_sizeof_vertices,
                          IVec2 render_texel_dimensions_2d) {
    if (!ptr_sizeof_vertices) {
        fprintf(stderr, "new_vertices: invalid ptr_sizeof_vertices.");
        return nullptr;
    }
    int number_of_vertices = render_texel_dimensions_2d.width
                             *render_texel_dimensions_2d.height;
    *ptr_sizeof_vertices = number_of_vertices*sizeof(Vec4);
    Vec4 *vertices = new Vec4[number_of_vertices];
    if (!vertices) {
        fprintf(stderr, "new_vertices: unable to allocate vertices.");
        return nullptr;
    }
    // In terms of the 2D texture, vertices are ordered as a stack of
    // ascending columns, starting from the origin and 
    // ending at the upper right corner. 
    for (int i = 0; i < render_texel_dimensions_2d.width; i++) {
        for (int j = 0; j < render_texel_dimensions_2d.height; j++) {
            int w = render_texel_dimensions_2d.width;
            int h = render_texel_dimensions_2d.height;
            float x = ((float)i + 0.5)/(float)w;
            float y = ((float)j + 0.5)/(float)h;
            // if (i == 255*64)
            //     fprintf(stdout, "%f, %f\n", x, y);
            vertices[j + i*h].x = x;
            vertices[j + i*h].y = y;
            vertices[j + i*h].z = 0.0;
            vertices[j + i*h].w = 1.0;
        }
    }
    return vertices;
}

/* Initialize the programs. */
void VolumeRender::init_programs() {
    // Program for sampling from the texture
    // that stores the 3D data
    this->programs.sample_volume = make_quad_program(
        "./shaders/sample-vol3d.frag"
    );
    // Program for showing the volume data
    this->programs.show_volume = make_program(
        "./shaders/show-vol3d.vert", 
        "./shaders/show-vol3d.frag");

    this->programs.gradient = make_quad_program(
        "./shaders/gradient3d.frag"
    );
}

/* Initialize all of the frames. */
void VolumeRender::init_frames() {

    // Create RGBA32F quads with the same dimensions as the sample
    struct TextureParams tex_params_sample_32f {
        .format=GL_RGBA32F, 
        .width=this->sample_texel_dimensions_2d.width,
        .height=this->sample_texel_dimensions_2d.height,
        .generate_mipmap=1,
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
    int sizeof_vertices {};
    Vec4 *vertices = new_vertices(&sizeof_vertices, 
                                  this->render_texel_dimensions_2d);
    
    // Construct elements
    int *elements = new_elements(&this->sizeof_elements,
                                 this->render_texel_dimensions_2d, 
                                 this->render_texel_dimensions_3d);
    
    if (vertices == nullptr || elements == nullptr)
        return;

    struct TextureParams tex_params_view_16f_mipmap_filter {
        .format=GL_RGBA16F, 
        .width=this->view_dimensions.width,
        .height=this->view_dimensions.height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR_MIPMAP_LINEAR, 
        .mag_filter=GL_LINEAR_MIPMAP_LINEAR,
    };
    this->frames.out = new_frame(&tex_params_view_16f_mipmap_filter, 
                                 (float *)vertices,
                                 sizeof_vertices,
                                 elements, this->sizeof_elements);

    // delete [] vertices;
    // delete [] elements;
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

static const char ATTRIBUTE_NAME[] = "uvIndex";
/* static const struct VertexParam VERTEX_PARAMS[2] = {
    {.name=(char *)&ATTRIBUTE_NAME[0], .size=4, .type=GL_FLOAT, 
        .normalized=GL_FALSE,
        .stride=4*sizeof(float), .offset=0}
};*/

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
                             int sizeof_elements,
                             const Quaternion &rotation,
                             const Quaternion &debug_rotation,
                             const IVec3 &render_texel_dimensions_3d,
                             const IVec2 &render_texel_dimensions_2d) {
    bind_frame(dst, show_vol_program);
    struct VertexParam vertex_params[2] = {
        {.name=(char *)&ATTRIBUTE_NAME[0], .size=4, .type=GL_FLOAT, 
        .normalized=GL_FALSE,
        .stride=4*sizeof(float), .offset=0}
    };
    set_vertex_attributes(vertex_params, 1);
    // Set vertex uniforms
    set_vec4_uniform("debugRotation",
                     debug_rotation.x,
                     debug_rotation.y,
                     debug_rotation.z,
                     debug_rotation.w);
    set_float_uniform("scale", 1.0);
    set_ivec3_uniform("texelDimensions3D",
                      render_texel_dimensions_3d.width,
                      render_texel_dimensions_3d.height,
                      render_texel_dimensions_3d.length);
    set_ivec2_uniform("texelDimensions2D",
                      render_texel_dimensions_2d.width,
                      render_texel_dimensions_2d.height);
    // Set fragment uniforms
    set_vec4_uniform("rotation", rotation.x, rotation.y,
                     rotation.z, rotation.w);
    set_sampler2D_uniform("gradientTex", sample_grad);
    set_sampler2D_uniform("densityTex", sample_vol);
    /* glDrawArrays(GL_LINES, 0, 
                 render_texel_dimensions_2d.width
                 *render_texel_dimensions_2d.height);*/
    glDrawElements(GL_TRIANGLES, sizeof_elements, GL_UNSIGNED_INT, 0);
    unbind();
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


    // Take the gradient of the volume data. Done in high precision float.
    gradient(this->frames.gradient, volume_data.get_frame_id(),
             0, this->programs.gradient, 
             2, 0, 0, 3, 
             this->sample_texel_dimensions_3d, 
             this->sample_texel_dimensions_2d);
    
    // Half precision gradient data
    tex_copy(this->frames.gradient_half_precision, this->frames.gradient);
    
    glViewport(
        0, 0, 
        render_texel_dimensions_2d.width,
        render_texel_dimensions_2d.height);

    // Sample the volume data so that it could be written to 
    // the render texture.
    sample_volume_data(this->frames.sample_volume, 
                       this->frames.vol_half_precision, 
                       this->programs.sample_volume, 
                       view_scale, 
                       rotation,
                       this->render_texel_dimensions_3d,
                       this->sample_texel_dimensions_3d,
                       this->sample_texel_dimensions_2d);
    // Sample the gradient data to its own render texture.
    sample_volume_data(this->frames.sample_grad, 
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
                        this->sizeof_elements,
                        rotation,
                        this->debug_rotation,
                        render_texel_dimensions_3d, 
                        render_texel_dimensions_2d);

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClear(GL_STENCIL_BUFFER_BIT);

    }
    /* if (!depth_test_prev_enabled)
        glDisable(GL_DEPTH_TEST);*/

    glDisable(GL_BLEND);

    Texture2DData t = Texture2DData(
        HALF_FLOAT4, 
        this->view_dimensions.width, 
        this->view_dimensions.height,
        true, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_NEAREST, GL_NEAREST);
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