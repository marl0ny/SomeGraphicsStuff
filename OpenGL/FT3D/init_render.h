#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERFACE_
#define _INTERFACE_


void init();

struct RenderParams {
    int user_use;
    int view_mode;
    int inc_mode1;
    int inc_mode2;
    int move_x, move_y, move_z;
    double x, y, dx, dy;
    double scroll;
};

void render(const struct RenderParams *render_params);


#endif

#ifdef __cplusplus
}
#endif