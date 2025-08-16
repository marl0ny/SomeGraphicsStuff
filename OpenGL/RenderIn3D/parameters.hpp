#include "gl_wrappers.hpp"

namespace sim_2d {

#ifndef _PARAMETERS_
#define _PARAMETERS_

struct Button {};

struct UploadImage {};

typedef std::string Label;

typedef bool BoolRecord;

typedef std::vector<std::string> EntryBoxes;

struct SelectionList {
    int selected;
    std::vector<std::string> options;
};

struct LineDivider {};

struct NotUsed {};

struct SimParams {
    float t = (float)(0.0F);
    float brightness = (float)(10.0F);
    Vec3 simulationDimensions3D = (Vec3)(Vec3 {.ind={128.0, 128.0, 128.0}});
    IVec3 dataTexelDimensions3D = (IVec3)(IVec3 {.ind={128, 128, 128}});
    SelectionList presetFunctionsDropdown = SelectionList{2, {"exp(-0.5*x^2/(10.0)^2)*sin(z/4.0)*sin(y/4.0)/(z*y)", "20.0*exp(0.0-0.5*((x/(sx*10.0))^2 + (y/(sy*10.0))^2))", "(x + i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6", "exp(-0.5*((x/(sx*10.0))^2 + (y/(sy*15.0))^2 + (z/(sz*10.0))^2))", "a*sin(x/10)*sin(y/10)*sin(z/10)", "step(sqrt(x^2 + y^2 + z^2) - 80)", "1 - step(x - 30) - step(-x - 30)", "abs(cos(k*x*y*z^2/1500000))^100", "log(abs(x/10))*log(abs(y/10))*log(abs(z/10))/10", "cos(10*x*y*z/100000)^3", "exp(-sqrt((x/5)^2 + (y/5)^2 + (z/5)^2))*(z + x)", "exp(-0.5*((x-x0)^2 + (y-y0)^2 + (z - z0)^2)/(s*15)^2)*exp(-i*(nx*x/50 + ny*y/50 + nz*z/50))", "(x+ i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6*exp(-f*i*t)", "exp(-0.5*z^2/(sz*4)^2) - exp(-0.5*y^2/(sy*4)^2) - i*exp(-0.5*x^2/(sx*4)^2)", "(x+i*y)^18/(x^2 + y^2)*exp(-(x^2+y^2 + z^2)/100)*(z/depth)^16*exp(-i*f*t)"}};
    EntryBoxes userTextEntry = EntryBoxes{"0"};
    SelectionList visualizationSelect = SelectionList{0, {"Volume render", "Three orthogonal planar slices", "Vector field"}};
    LineDivider volRenderLineDiv = LineDivider{};
    Label volumeRenderTitle = Label{};
    float alphaBrightness = (float)(1.0F);
    float colorBrightness = (float)(1.0F);
    IVec3 volumeTexelDimensions3D = (IVec3)(IVec3 {.ind={128, 128, 192}});
    float noiseScale = (float)(0.25F);
    LineDivider planarSlicesLineDiv = LineDivider{};
    Label planarSlicesLabel = Label{};
    Vec3 planarNormCoordOffsets = (Vec3)(Vec3 {.ind={0.5, 0.5, 0.5}});
    LineDivider arrows3DLineDiv = LineDivider{};
    Label arrows3DLabel = Label{};
    IVec3 arrowDimensions = (IVec3)(IVec3 {.ind={32, 32, 32}});
    int dummyValue = (int)(0);
    enum {
        T=0,
        BRIGHTNESS=1,
        SIMULATION_DIMENSIONS3_D=2,
        DATA_TEXEL_DIMENSIONS3_D=3,
        PRESET_FUNCTIONS_DROPDOWN=4,
        USER_TEXT_ENTRY=5,
        VISUALIZATION_SELECT=6,
        VOL_RENDER_LINE_DIV=7,
        VOLUME_RENDER_TITLE=8,
        ALPHA_BRIGHTNESS=9,
        COLOR_BRIGHTNESS=10,
        VOLUME_TEXEL_DIMENSIONS3_D=11,
        NOISE_SCALE=12,
        PLANAR_SLICES_LINE_DIV=13,
        PLANAR_SLICES_LABEL=14,
        PLANAR_NORM_COORD_OFFSETS=15,
        ARROWS3_D_LINE_DIV=16,
        ARROWS3_D_LABEL=17,
        ARROW_DIMENSIONS=18,
        DUMMY_VALUE=19,
    };
    void set(int enum_val, Uniform val) {
        switch(enum_val) {
            case T:
            t = val.f32;
            break;
            case BRIGHTNESS:
            brightness = val.f32;
            break;
            case SIMULATION_DIMENSIONS3_D:
            simulationDimensions3D = val.vec3;
            break;
            case DATA_TEXEL_DIMENSIONS3_D:
            dataTexelDimensions3D = val.ivec3;
            break;
            case ALPHA_BRIGHTNESS:
            alphaBrightness = val.f32;
            break;
            case COLOR_BRIGHTNESS:
            colorBrightness = val.f32;
            break;
            case VOLUME_TEXEL_DIMENSIONS3_D:
            volumeTexelDimensions3D = val.ivec3;
            break;
            case NOISE_SCALE:
            noiseScale = val.f32;
            break;
            case PLANAR_NORM_COORD_OFFSETS:
            planarNormCoordOffsets = val.vec3;
            break;
            case ARROW_DIMENSIONS:
            arrowDimensions = val.ivec3;
            break;
            case DUMMY_VALUE:
            dummyValue = val.i32;
            break;
        }
    }
    Uniform get(int enum_val) const {
        switch(enum_val) {
            case T:
            return {(float)t};
            case BRIGHTNESS:
            return {(float)brightness};
            case SIMULATION_DIMENSIONS3_D:
            return {(Vec3)simulationDimensions3D};
            case DATA_TEXEL_DIMENSIONS3_D:
            return {(IVec3)dataTexelDimensions3D};
            case ALPHA_BRIGHTNESS:
            return {(float)alphaBrightness};
            case COLOR_BRIGHTNESS:
            return {(float)colorBrightness};
            case VOLUME_TEXEL_DIMENSIONS3_D:
            return {(IVec3)volumeTexelDimensions3D};
            case NOISE_SCALE:
            return {(float)noiseScale};
            case PLANAR_NORM_COORD_OFFSETS:
            return {(Vec3)planarNormCoordOffsets};
            case ARROW_DIMENSIONS:
            return {(IVec3)arrowDimensions};
            case DUMMY_VALUE:
            return {(int)dummyValue};
        }
        return Uniform(0);
    }
    void set(int enum_val, int index, std::string val) {
        switch(enum_val) {
            case USER_TEXT_ENTRY:
            userTextEntry[index] = val;
            break;
            case VOLUME_RENDER_TITLE:
            volumeRenderTitle = val;
            break;
            case PLANAR_SLICES_LABEL:
            planarSlicesLabel = val;
            break;
            case ARROWS3_D_LABEL:
            arrows3DLabel = val;
            break;
        }
    }
};
#endif
}
