#include <metal_stdlib>
using namespace metal;

struct VertexToFragmentData {
    float4 position [[position]];
    float2 uv;
};

vertex VertexToFragmentData vertex_func(constant float3 *vertices [[buffer(0)]],
                          uint index [[ vertex_id ]]) {
    VertexToFragmentData data = {
        .position = float4(vertices[index], 1.0),
        .uv = float2(vertices[index].x, -vertices[index].y)/2.0 + 0.5
    };
    return data;
}

fragment float4 wave_step(VertexToFragmentData data [[ stage_in ]],
                          texture2d<float> tex1 [[texture(0)]],
                          texture2d<float> tex2 [[texture(1)]]) {
    float2 uv = data.uv;
    sampler texSampler (mag_filter::nearest, min_filter::nearest,
                        s_address::repeat, t_address::repeat);
    float4 u = tex2.sample(texSampler, uv, int2(0, 1));
    float4 d = tex2.sample(texSampler, uv, int2(0, -1));
    float4 l = tex2.sample(texSampler, uv, int2(-1, 0));
    float4 r = tex2.sample(texSampler, uv, int2(1, 0));
    float4 c = tex1.sample(texSampler, uv);
    return (u + d + l + r)/2.0F - c;
}

struct MainFragmentViewUniforms {
    union {
        struct {
            float2 dimensions;
        };
        struct {
            float width;
            float height;
        };
    };
};

fragment float4 main_fragment_view(VertexToFragmentData data [[ stage_in ]],
                                   texture2d<float> tex [[texture(2)]],
                                   constant MainFragmentViewUniforms *uniforms [[buffer(3)]]) {
    float2 uv = data.uv;
    sampler texSampler(mag_filter::linear, min_filter::linear, s_address::clamp_to_zero, t_address::clamp_to_zero);
    if (uniforms->width == uniforms->height) {
        return tex.sample(texSampler, uv);
    } else {
        float scale_u = uniforms->width/uniforms->height;
        float offset_u = (scale_u - 1.0)/2.0;
        float scale_v = uniforms->height/uniforms->width;
        float offset_v = (scale_v - 1.0)/2.0;
        if (uniforms->width > uniforms->height) {
            return tex.sample(texSampler, float2(uv[0]*scale_u - offset_u, uv[1]));
        } else {
            return tex.sample(texSampler, float2(uv[0], uv[1]*scale_v - offset_v));
        }
    }
}

fragment float4 add2_rgb(VertexToFragmentData data [[ stage_in ]],
                         texture2d<float> tex1 [[texture(1)]],
                         texture2d<float> tex2 [[texture(2)]]) {
    float2 uv = data.uv;
    sampler texSampler(mag_filter::linear, min_filter::linear);
    return float4(tex1.sample(texSampler, uv).xyz + tex2.sample(texSampler, uv).xyz, 1.0);
}

fragment float4 copy(VertexToFragmentData data [[ stage_in ]], texture2d<float> tex [[texture(0)]]) {
    float2 uv = data.uv;
    sampler texSampler(mag_filter::linear, min_filter::linear);
    return tex.sample(texSampler, uv);
}

fragment float4 zero(VertexToFragmentData data [[ stage_in ]]) {
    return float4(0.0, 0.0, 0.0, 0.0);
}

struct GaussianShaderUniforms {
    float4 location;
    float4 colour;
};

fragment float4 gaussian(VertexToFragmentData data [[ stage_in ]],
                         constant GaussianShaderUniforms* gaussianShaderData [[buffer(1)]]) {
    float2 uv = data.uv;
    float2 r0 = gaussianShaderData->location.xy;
    float bx = r0.x, by = r0.y;
    float x = uv.x - bx, y = uv.y - by;
    float sigmaX2 = 0.02*0.02, sigmaY2 = 0.02*0.02;
    float val = 10.0*exp(-0.5*x*x/sigmaX2 - 0.5*y*y/sigmaY2);
    float4 colour = gaussianShaderData->colour;
    return float4(val*colour.r, val*colour.g, val*colour.b, colour.a);
}


fragment float4 fragment_func2(VertexToFragmentData data [[ stage_in ]]) {
    float2 uv = data.uv;
    return float4(metal::cos(uv.x), uv.y, -uv.y, 0.5);
}
