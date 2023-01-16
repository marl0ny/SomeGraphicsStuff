#ifndef _INTERFACE_
#define _INTERFACE_


void init();

struct RenderParams {
    int user_use;
    double x, y, dx, dy;
    double scroll;
};

void render(const struct RenderParams *render_params);


#endif