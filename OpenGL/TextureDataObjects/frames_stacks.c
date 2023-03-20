#include "frames_stacks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 20

struct FramesStack {
    frame_id frames[MAX_FRAMES];
    int total_count;
    int active_count;
};

#define MAX_FRAME_STACKS 20

static struct {
    int count;
    struct TextureParams tex_type[MAX_FRAME_STACKS];
    struct FramesStack ind[MAX_FRAME_STACKS];
    /*union {
        struct FramesStack ind[5];
        struct {
            struct FramesStack rgba32f;
            struct FramesStack rgba16f;
            struct FramesStack rg32f;
            struct FramesStack rg16f;
            struct FramesStack red32f;
            struct FramesStack red16f;
        };
    };*/
} s_all_stacks;

static struct FramesStack *get_stack_frame(const struct TextureParams *t) {
    if (t == NULL) {
        fprintf(stderr, "get_stack_frame: null input.\n");
    }
    for (int i = 0; i < s_all_stacks.count; i++) {
        int f = memcmp((void *)t, (void *)&s_all_stacks.tex_type[i],
                       sizeof(struct TextureParams));
        if (f == 0)
            return (struct FramesStack *)&s_all_stacks.ind[i];
    }
    return NULL;
}

frame_id activate_frame(const struct TextureParams *tex_params) {
    if (tex_params == NULL) {
        fprintf(stderr, "activate_frame: null input.\n");
    }
    struct FramesStack *s  = get_stack_frame(tex_params);
    if (s == NULL) {
        if (s_all_stacks.count == MAX_FRAME_STACKS) {
            fprintf(stderr,
                    "activate_frame: "
                    "not enough space left for new texture frames\n");
            return 0;
        }
        memcpy((void *)&s_all_stacks.tex_type[s_all_stacks.count],
               tex_params, sizeof(struct TextureParams));
        s = &s_all_stacks.ind[s_all_stacks.count];
        s_all_stacks.count++;
    }
    int total_count = s->total_count, active_count = s->active_count;
    if (total_count == active_count) {
        if (total_count == MAX_FRAMES) {
            fprintf(stderr,
                    "activate_frame: not enough space left "
                    "for texture frame of same type.\n");
            return 0;
        }
        s->frames[s->total_count++] = new_quad(tex_params);
        return s->frames[s->active_count++];
    } else if (active_count < total_count) {
        return s->frames[s->active_count++];
    }
    return 0;
}

static void swap2(frame_id *a, frame_id *b) {
    frame_id tmp = *a;
    *a = *b;
    *b = tmp;
}

static void make_active_frames_contiguous(struct FramesStack *s,
                                          int non_active_index,
                                          int active_count) {
    for (int i = non_active_index; i < active_count ; i++) {
        swap2(&s->frames[i], &s->frames[i+1]);
    }
}

void deactivate_frame(const struct TextureParams *tex_params,
                      frame_id id_of_frame) {
    // printf("Deactivating frame %d.\n", id_of_frame);
    struct FramesStack *s = get_stack_frame(tex_params);
    // struct FramesStack *s  = &s_all_stacks.ind[type];
    int active_count = s->active_count;
    for (int i = active_count - 1; i >= 0; i--) {
        if (id_of_frame == s->frames[i]) {
            s->active_count--;
            if (i != (active_count - 1))
                make_active_frames_contiguous(s, i, s->active_count);
            return;
        }
    }
}

void debug_print_frames_stack(const struct TextureParams *tex_params) {
    struct FramesStack *s = get_stack_frame(tex_params);
    if (s == NULL)
        return;
    printf("ID     ");
    for (int i = 0; i < MAX_FRAMES; i++) {
        printf("%d ", s->frames[i]);
    }
    puts("");
    printf("Active ");
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (i < s->active_count)
            printf("1 ");
        else if (i == s->active_count)
            printf("* ");
        else
            printf("  ");
    }
    puts("");
    printf("Total  ");
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (i < s->total_count)
            printf("1 ");
        else if (i == s->total_count)
            printf("* ");
        else
            printf("  ");
    }
    puts("");
}

/* int main() {
    struct TextureParams tex_param1 = {
        .format=GL_RGBA32F, .width=512, .height=512,
        .generate_mipmap=1,
        .wrap_s=GL_REPEAT, .wrap_t=GL_REPEAT,
        .mag_filter=GL_LINEAR, .min_filter=GL_LINEAR,
    };
    struct TextureParams tex_param2 = {
        .format=GL_RGBA16F, .width=512, .height=512,
        .generate_mipmap=1,
        .wrap_s=GL_REPEAT, .wrap_t=GL_REPEAT,
        .mag_filter=GL_LINEAR, .min_filter=GL_LINEAR,
    };
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    deactivate_frame(&tex_param1, 1);
    deactivate_frame(&tex_param1, 2);
    deactivate_frame(&tex_param1, 3);
    deactivate_frame(&tex_param1, 4);
    debug_print_frames_stack(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    activate_frame(&tex_param1);
    debug_print_frames_stack(&tex_param1);
    deactivate_frame(&tex_param1, 2);
    deactivate_frame(&tex_param1, 1);
    deactivate_frame(&tex_param1, 2);
    deactivate_frame(&tex_param1, 1);
    debug_print_frames_stack(&tex_param1);
    activate_frame(&tex_param2);
    activate_frame(&tex_param2);
    activate_frame(&tex_param2);
    debug_print_frames_stack(&tex_param2);
    return 0;
    }*/

#undef MAX_FRAMES
#undef MAX_FRAME_STACKS
