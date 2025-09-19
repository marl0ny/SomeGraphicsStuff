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
    Label fpsLabel = Label{};
    Label totalELabel = Label{};
    Label driftELabel = Label{};
    int particleCount = (int)(16384);
    IVec2 cellGridDimensions2D = (IVec2)(IVec2 {.ind={64, 64}});
    Vec2 simDimensions2D = (Vec2)(Vec2 {.ind={1.7, 1.7}});
    int stepsPerFrame = (int)(1);
    LineDivider lineDivDT1 = LineDivider{};
    Label timeStepConfigTitle = Label{};
    float requestedDt = (float)(5e-08F);
    float limitTimeStepAgressiveness = (float)(250.0F);
    float dt = (float)(5e-08F);
    Label dtLabel = Label{};
    LineDivider lineDivDT2 = LineDivider{};
    Label potentialConfigTitle = Label{};
    float sigma = (float)(0.0075F);
    Label sigmaLabel = Label{};
    float epsilon = (float)(20000000.0F);
    LineDivider lineDiv3 = LineDivider{};
    float gForce = (float)(-10000000.0F);
    float particleBrightness = (float)(1.0F);
    float wallForce = (float)(100000000000000.0F);
    float totalEnergy = (float)(0.0F);
    int nSteps = (int)(0);
    int nFrames = (int)(0);
    Label simulationDomainLabel = Label{};
    Label xRangeLabel = Label{};
    Label yRangeLabel = Label{};
    SelectionList particleCountSelector = SelectionList{8, {"64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768"}};
    bool showCellsByParticleCount = (bool)(false);
    int neighbourCellCountForForceComputation = (int)(3);
    SelectionList neighbourCellCountForForceComputationSelector = SelectionList{1, {"3", "8"}};
    int dummyValue = (int)(16384);
    enum {
        FPS_LABEL=0,
        TOTAL_E_LABEL=1,
        DRIFT_E_LABEL=2,
        PARTICLE_COUNT=3,
        CELL_GRID_DIMENSIONS2_D=4,
        SIM_DIMENSIONS2_D=5,
        STEPS_PER_FRAME=6,
        LINE_DIV_D_T1=7,
        TIME_STEP_CONFIG_TITLE=8,
        REQUESTED_DT=9,
        LIMIT_TIME_STEP_AGRESSIVENESS=10,
        DT=11,
        DT_LABEL=12,
        LINE_DIV_D_T2=13,
        POTENTIAL_CONFIG_TITLE=14,
        SIGMA=15,
        SIGMA_LABEL=16,
        EPSILON=17,
        LINE_DIV3=18,
        G_FORCE=19,
        PARTICLE_BRIGHTNESS=20,
        WALL_FORCE=21,
        TOTAL_ENERGY=22,
        N_STEPS=23,
        N_FRAMES=24,
        SIMULATION_DOMAIN_LABEL=25,
        X_RANGE_LABEL=26,
        Y_RANGE_LABEL=27,
        PARTICLE_COUNT_SELECTOR=28,
        SHOW_CELLS_BY_PARTICLE_COUNT=29,
        NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION=30,
        NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION_SELECTOR=31,
        DUMMY_VALUE=32,
    };
    void set(int enum_val, Uniform val) {
        switch(enum_val) {
            case PARTICLE_COUNT:
            particleCount = val.i32;
            break;
            case CELL_GRID_DIMENSIONS2_D:
            cellGridDimensions2D = val.ivec2;
            break;
            case SIM_DIMENSIONS2_D:
            simDimensions2D = val.vec2;
            break;
            case STEPS_PER_FRAME:
            stepsPerFrame = val.i32;
            break;
            case REQUESTED_DT:
            requestedDt = val.f32;
            break;
            case LIMIT_TIME_STEP_AGRESSIVENESS:
            limitTimeStepAgressiveness = val.f32;
            break;
            case DT:
            dt = val.f32;
            break;
            case SIGMA:
            sigma = val.f32;
            break;
            case EPSILON:
            epsilon = val.f32;
            break;
            case G_FORCE:
            gForce = val.f32;
            break;
            case PARTICLE_BRIGHTNESS:
            particleBrightness = val.f32;
            break;
            case WALL_FORCE:
            wallForce = val.f32;
            break;
            case TOTAL_ENERGY:
            totalEnergy = val.f32;
            break;
            case N_STEPS:
            nSteps = val.i32;
            break;
            case N_FRAMES:
            nFrames = val.i32;
            break;
            case SHOW_CELLS_BY_PARTICLE_COUNT:
            showCellsByParticleCount = val.b32;
            break;
            case NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION:
            neighbourCellCountForForceComputation = val.i32;
            break;
            case DUMMY_VALUE:
            dummyValue = val.i32;
            break;
        }
    }
    Uniform get(int enum_val) const {
        switch(enum_val) {
            case PARTICLE_COUNT:
            return {(int)particleCount};
            case CELL_GRID_DIMENSIONS2_D:
            return {(IVec2)cellGridDimensions2D};
            case SIM_DIMENSIONS2_D:
            return {(Vec2)simDimensions2D};
            case STEPS_PER_FRAME:
            return {(int)stepsPerFrame};
            case REQUESTED_DT:
            return {(float)requestedDt};
            case LIMIT_TIME_STEP_AGRESSIVENESS:
            return {(float)limitTimeStepAgressiveness};
            case DT:
            return {(float)dt};
            case SIGMA:
            return {(float)sigma};
            case EPSILON:
            return {(float)epsilon};
            case G_FORCE:
            return {(float)gForce};
            case PARTICLE_BRIGHTNESS:
            return {(float)particleBrightness};
            case WALL_FORCE:
            return {(float)wallForce};
            case TOTAL_ENERGY:
            return {(float)totalEnergy};
            case N_STEPS:
            return {(int)nSteps};
            case N_FRAMES:
            return {(int)nFrames};
            case SHOW_CELLS_BY_PARTICLE_COUNT:
            return {(bool)showCellsByParticleCount};
            case NEIGHBOUR_CELL_COUNT_FOR_FORCE_COMPUTATION:
            return {(int)neighbourCellCountForForceComputation};
            case DUMMY_VALUE:
            return {(int)dummyValue};
        }
        return Uniform(0);
    }
    void set(int enum_val, int index, std::string val) {
        switch(enum_val) {
            case FPS_LABEL:
            fpsLabel = val;
            break;
            case TOTAL_E_LABEL:
            totalELabel = val;
            break;
            case DRIFT_E_LABEL:
            driftELabel = val;
            break;
            case TIME_STEP_CONFIG_TITLE:
            timeStepConfigTitle = val;
            break;
            case DT_LABEL:
            dtLabel = val;
            break;
            case POTENTIAL_CONFIG_TITLE:
            potentialConfigTitle = val;
            break;
            case SIGMA_LABEL:
            sigmaLabel = val;
            break;
            case SIMULATION_DOMAIN_LABEL:
            simulationDomainLabel = val;
            break;
            case X_RANGE_LABEL:
            xRangeLabel = val;
            break;
            case Y_RANGE_LABEL:
            yRangeLabel = val;
            break;
        }
    }
};
#endif
}
