#include "tex_sort.hpp"
#include <OpenGL/OpenGL.h>


using namespace sort;

sort::Frames::Frames(int w, int h) :
    sort_texture(TextureParams{
        .format=GL_RGBA32F,
        .width=(unsigned int)w,
        .height=(unsigned int)h,
        .generate_mipmap=1,
        .min_filter=GL_NEAREST, .mag_filter=GL_NEAREST,
        .wrap_s=GL_REPEAT, .wrap_t=GL_REPEAT
    }),
    tmp(sort_texture), 
    sort_iter {
        Quad(sort_texture), Quad(sort_texture)} 
    {

}

void sort::Frames::reset_dimensions(int w, int h) {
    this->sort_texture.width = w;
    this->sort_texture.height = h;
    this->tmp.reset(this->sort_texture);
    this->sort_iter[0].reset(this->sort_texture);
    this->sort_iter[1].reset(this->sort_texture);
}

sort::Programs::Programs() {
    this->sort4 = Quad::make_program_from_path(
        "./shaders/sort/sort4.frag");
    this->bitonic_sort.size4 = Quad::make_program_from_path(
        "./shaders/sort/bitonic-sort4.frag");
    this->bitonic_sort.part1 = Quad::make_program_from_path(
        "./shaders/sort/bitonic-sort-iter-part1.frag");
    this->bitonic_sort.part2 = Quad::make_program_from_path(
        "./shaders/sort/bitonic-sort-iter-part2.frag");
}

Sort::Sort(int w, int h): m_programs(), m_frames(Frames(w, h)) {}

Sort::Sort(IVec2 d_2d): m_programs(), m_frames(Frames(d_2d[0], d_2d[1])) {}

void Sort::operator()(Quad &dst, const Quad &src) {
    int size = dst.width()*dst.height();
    IVec2 tex_dimensions2d {.ind{
        (int)src.width(), (int)src.height()}};
    m_frames.tmp.draw(
        m_programs.sort4,
        {
            {"tex", &src},
            {"texDimensions2D", tex_dimensions2d}
        }
    );
    for (int block_size = 8; block_size <= size; block_size *= 2) {
        if (block_size < size)
            this->bitonic(m_frames.tmp, m_frames.tmp, block_size);
        else
            this->bitonic(dst, m_frames.tmp, size);
    }
}

void Sort::bitonic(Quad &dst, const Quad &src) {
    int size = dst.width()*dst.height();
    this->bitonic(dst, src, size);
}

void Sort::bitonic(Quad &dst, const Quad &src, int size) {
    IVec2 tex_dimensions2d {.ind{
        (int)src.width(), (int)src.height()}};
    if (size == 4) {
        dst.draw(
            m_programs.sort4,
            {
                {"tex", &src},
                {"texDimensions2D", tex_dimensions2d}});
        return;
    }
    for (int block_size = size; block_size > 4; block_size /= 2) {
        const Quad *last_ptr = (size == block_size)? &src: &m_frames.sort_iter[0];
        m_frames.sort_iter[1].draw(
            m_programs.bitonic_sort.part1,
            {
                {"tex", last_ptr},
                {"texDimensions2D", tex_dimensions2d},
                {"flipOrderSize", int(size)},
                {"blockSize", int(block_size)}
            }
        );
        m_frames.sort_iter[0].draw(
            m_programs.bitonic_sort.part2,
            {
                {"tex", &m_frames.sort_iter[1]},
                {"texDimensions2D", tex_dimensions2d},
                {"blockSize", int(block_size)}
            }
        );
    }
    dst.draw(
        m_programs.bitonic_sort.size4,
        {
            {"tex", &m_frames.sort_iter[0]},
            {"texDimensions2D", tex_dimensions2d},
            {"flipOrderSize", (int)size}});

}

void Sort::reset_dimensions(int w, int h) {
    m_frames.reset_dimensions(w, h);
}

void Sort::reset_dimensions(IVec2 d_2d) {
    m_frames.reset_dimensions(d_2d[0], d_2d[1]);
}
