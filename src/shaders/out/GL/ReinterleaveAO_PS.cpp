static const char* g_ReinterleaveAO_PS_ENABLE_BLUR_0_GL =
"#version 150\n"
"struct vec1 {\n"
"	float x;\n"
"};\n"
"struct uvec1 {\n"
"	uint x;\n"
"};\n"
"struct ivec1 {\n"
"	int x;\n"
"};\n"
"layout(std140) uniform;\n"
"uniform GlobalConstantBuffer {\n"
" 	uvec4 GlobalConstantBuffer_0;\n"
"	vec2 GlobalConstantBuffer_1;\n"
"	vec2 GlobalConstantBuffer_2;\n"
"	vec2 GlobalConstantBuffer_3;\n"
"	vec2 GlobalConstantBuffer_4;\n"
"	float GlobalConstantBuffer_5;\n"
"	float GlobalConstantBuffer_6;\n"
"	float GlobalConstantBuffer_7;\n"
"	float GlobalConstantBuffer_8;\n"
"	float GlobalConstantBuffer_9;\n"
"	float GlobalConstantBuffer_10;\n"
"	float GlobalConstantBuffer_11;\n"
"	int GlobalConstantBuffer_12;\n"
"	float GlobalConstantBuffer_13;\n"
"	float GlobalConstantBuffer_14;\n"
"	float GlobalConstantBuffer_15;\n"
"	float GlobalConstantBuffer_16;\n"
"	float GlobalConstantBuffer_17;\n"
"	float GlobalConstantBuffer_18;\n"
"	float GlobalConstantBuffer_19;\n"
"	float GlobalConstantBuffer_20;\n"
"	vec2 GlobalConstantBuffer_21;\n"
"	float GlobalConstantBuffer_22;\n"
"	float GlobalConstantBuffer_23;\n"
"	float GlobalConstantBuffer_24;\n"
"	float GlobalConstantBuffer_25;\n"
"	int GlobalConstantBuffer_26;\n"
"	vec4 GlobalConstantBuffer_27[4];\n"
"	float GlobalConstantBuffer_28;\n"
"	float GlobalConstantBuffer_29;\n"
"};\n"
"uniform sampler2DArray g_t0;\n"
"vec4 Input0;\n"
"out  vec4 PixOutput0;\n"
"#define Output0 PixOutput0\n"
"vec4 Temp[2];\n"
"ivec4 Temp_int[2];\n"
"uvec4 Temp_uint[2];\n"
"void main()\n"
"{\n"
"    Input0.xy = gl_FragCoord.xy;\n"
"    Temp[0].xy = vec4(Input0.xyxx + -GlobalConstantBuffer_21.xyxx.xyxx).xy;\n"
"    Temp[0].xy = vec4(floor(Temp[0].xyxx)).xy;\n"
"    Temp[0].zw = vec4(abs(Temp[0].yyyx) * vec4(0.000000, 0.000000, 0.250000, 0.250000)).zw;\n"
"    Temp[0].xy = vec4(Temp[0].xyxx * vec4(0.250000, 0.250000, 0.000000, 0.000000)).xy;\n"
"    Temp_int[1].xy = ivec4(Temp[0].xyxx).xy;\n"
"    Temp[0].xy = vec4(fract(Temp[0].zwzz)).xy;\n"
"    Temp[0].x = vec4(dot((Temp[0].xyxx).xy, (vec4(16.000000, 4.000000, 0.000000, 0.000000)).xy)).x;\n"
"    Temp_int[1].z = int(Temp[0].x);\n"
"    Temp[1].w = vec4(0.000000).w;\n"
"    Temp[0].x = texelFetch(g_t0, ivec3((Temp_int[1]).xyz), 0).x;\n"
"    Temp[0].x = vec4(Temp[0].x).x;\n"
"    Temp[0].x = clamp(Temp[0].x, 0.0, 1.0);\n"
"    Temp[0].x = vec4(log2(Temp[0].x)).x;\n"
"    Temp[0].x = Temp[0].x * GlobalConstantBuffer_11;\n"
"    Output0 = vec4(exp2(Temp[0].xxxx));\n"
"    return;\n"
"}\n"
;
static const char* g_ReinterleaveAO_PS_ENABLE_BLUR_1_GL =
"#version 150\n"
"struct vec1 {\n"
"	float x;\n"
"};\n"
"struct uvec1 {\n"
"	uint x;\n"
"};\n"
"struct ivec1 {\n"
"	int x;\n"
"};\n"
"layout(std140) uniform;\n"
"uniform sampler2DArray g_t0;\n"
"uniform sampler2D g_t1;\n"
"vec4 Input0;\n"
" in  vec4 VtxGeoOutput1;\n"
"vec4 Input1;\n"
"out  vec4 PixOutput0;\n"
"#define Output0 PixOutput0\n"
"vec4 Temp[2];\n"
"ivec4 Temp_int[2];\n"
"uvec4 Temp_uint[2];\n"
"void main()\n"
"{\n"
"    Input0.xy = gl_FragCoord.xy;\n"
"    Input1 = VtxGeoOutput1;\n"
"    Temp[0].xy = vec4(floor(Input0.xyxx)).xy;\n"
"    Temp[0].zw = vec4(abs(Temp[0].yyyx) * vec4(0.000000, 0.000000, 0.250000, 0.250000)).zw;\n"
"    Temp[0].xy = vec4(Temp[0].xyxx * vec4(0.250000, 0.250000, 0.000000, 0.000000)).xy;\n"
"    Temp_int[1].xy = ivec4(Temp[0].xyxx).xy;\n"
"    Temp[0].xy = vec4(fract(Temp[0].zwzz)).xy;\n"
"    Temp[0].x = vec4(dot((Temp[0].xyxx).xy, (vec4(16.000000, 4.000000, 0.000000, 0.000000)).xy)).x;\n"
"    Temp_int[1].z = int(Temp[0].x);\n"
"    Temp[1].w = vec4(0.000000).w;\n"
"    Temp[0].x = texelFetch(g_t0, ivec3((Temp_int[1]).xyz), 0).x;\n"
"    Output0.x = vec4(Temp[0].x).x;\n"
"    Temp[0].x = (texture(g_t1, Input1.xy)).x;\n"
"    Output0.y = vec4(Temp[0].x).y;\n"
"    return;\n"
"}\n"
;

namespace Generated
{
    void ReinterleaveAO_PS::Create(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::ENABLE_BLUR_0].Create(Device, g_ReinterleaveAO_PS_ENABLE_BLUR_0_GL, sizeof(g_ReinterleaveAO_PS_ENABLE_BLUR_0_GL));
        m_Shader[ShaderPermutations::ENABLE_BLUR_1].Create(Device, g_ReinterleaveAO_PS_ENABLE_BLUR_1_GL, sizeof(g_ReinterleaveAO_PS_ENABLE_BLUR_1_GL));
    }

    void ReinterleaveAO_PS::Release(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::ENABLE_BLUR_0].Release(Device);
        m_Shader[ShaderPermutations::ENABLE_BLUR_1].Release(Device);
    }
}
