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
    float brightness = (float)(10.0F);
    int dummyValue = (int)(0);
    enum {
        BRIGHTNESS=0,
        DUMMY_VALUE=1,
    };
    void set(int enum_val, Uniform val) {
        switch(enum_val) {
            case BRIGHTNESS:
            brightness = val.f32;
            break;
            case DUMMY_VALUE:
            dummyValue = val.i32;
            break;
        }
    }
    Uniform get(int enum_val) const {
        switch(enum_val) {
            case BRIGHTNESS:
            return {(float)brightness};
            case DUMMY_VALUE:
            return {(int)dummyValue};
        }
        return Uniform(0);
    }
    void set(int enum_val, int index, std::string val) {
        switch(enum_val) {
        }
    }
};
#endif
}
