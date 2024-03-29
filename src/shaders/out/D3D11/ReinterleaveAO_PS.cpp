#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xy  
// TEXCOORD                 0   xy          1     NONE   float       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_TARGET                0   xyzw        0   TARGET   float   xyzw
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[8], immediateIndexed
dcl_resource_texture2darray (float,float,float,float) t0
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.xyzw
dcl_temps 2
add r0.xy, v0.xyxx, -cb0[7].xyxx
ftoi r0.xy, r0.xyxx
bfi r0.z, l(2), l(2), r0.y, l(0)
bfi r1.z, l(2), l(0), r0.x, r0.z
ishr r1.xy, r0.xyxx, l(2, 2, 0, 0)
mov r1.w, l(0)
ld_indexable(texture2darray)(float,float,float,float) r0.x, r1.xyzw, t0.xyzw
mov_sat r0.x, r0.x
log r0.x, r0.x
mul r0.x, r0.x, cb0[4].z
exp o0.xyzw, r0.xxxx
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_ReinterleaveAO_PS_ENABLE_BLUR_0_DEPTH_LAYER_COUNT_1_D3D11[] =
{
     68,  88,  66,  67,  50,  64, 
     90, 226, 170,  28, 158, 218, 
     73, 191,  66,  28,  80, 182, 
    224, 122,   1,   0,   0,   0, 
     96,   2,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0, 184,   0, 
      0,   0,  73,  83,  71,  78, 
     80,   0,   0,   0,   2,   0, 
      0,   0,   8,   0,   0,   0, 
     56,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   3,   0,   0, 
     68,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
     83,  86,  95,  80, 111, 115, 
    105, 116, 105, 111, 110,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171, 171, 171, 
     79,  83,  71,  78,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,  83,  86, 
     95,  84,  65,  82,  71,  69, 
     84,   0, 171, 171,  83,  72, 
     69,  88, 160,   1,   0,   0, 
     80,   0,   0,   0, 104,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  88,  64, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0, 100,  32,   0,   4, 
     50,  16,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   2,   0, 
      0,   0,   0,   0,   0,   9, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      0,   0,   0,   0,  70, 128, 
     32, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  27,   0,   0,   5, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11,  66,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
    140,   0,   0,  11,  66,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,   0,  10, 
     50,   0,  16,   0,   1,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  45,   0,   0, 137, 
      2,   2,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,  54,  32,   0,   5, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  47,   0, 
      0,   5,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,  25,   0,   0,   5, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1
};
#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xy  
// TEXCOORD                 0   xy          1     NONE   float       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_TARGET                0   xyzw        0   TARGET   float   xyzw
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[8], immediateIndexed
dcl_resource_texture2darray (float,float,float,float) t0
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.xyzw
dcl_temps 2
add r0.xy, v0.xyxx, -cb0[7].xyxx
ftoi r0.xy, r0.xyxx
bfi r0.z, l(2), l(2), r0.y, l(0)
bfi r1.z, l(2), l(0), r0.x, r0.z
ishr r1.xy, r0.xyxx, l(2, 2, 0, 0)
mov r1.w, l(0)
ld_indexable(texture2darray)(float,float,float,float) r0.x, r1.xyzw, t0.xyzw
mov_sat r0.x, r0.x
log r0.x, r0.x
mul r0.x, r0.x, cb0[4].z
exp o0.xyzw, r0.xxxx
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_ReinterleaveAO_PS_ENABLE_BLUR_0_DEPTH_LAYER_COUNT_2_D3D11[] =
{
     68,  88,  66,  67,  50,  64, 
     90, 226, 170,  28, 158, 218, 
     73, 191,  66,  28,  80, 182, 
    224, 122,   1,   0,   0,   0, 
     96,   2,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0, 184,   0, 
      0,   0,  73,  83,  71,  78, 
     80,   0,   0,   0,   2,   0, 
      0,   0,   8,   0,   0,   0, 
     56,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   3,   0,   0, 
     68,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
     83,  86,  95,  80, 111, 115, 
    105, 116, 105, 111, 110,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171, 171, 171, 
     79,  83,  71,  78,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,  83,  86, 
     95,  84,  65,  82,  71,  69, 
     84,   0, 171, 171,  83,  72, 
     69,  88, 160,   1,   0,   0, 
     80,   0,   0,   0, 104,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  88,  64, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0, 100,  32,   0,   4, 
     50,  16,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   2,   0, 
      0,   0,   0,   0,   0,   9, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      0,   0,   0,   0,  70, 128, 
     32, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  27,   0,   0,   5, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11,  66,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
    140,   0,   0,  11,  66,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,   0,  10, 
     50,   0,  16,   0,   1,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  45,   0,   0, 137, 
      2,   2,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,  54,  32,   0,   5, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  47,   0, 
      0,   5,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,  25,   0,   0,   5, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1
};
#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xy  
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_TARGET                0   xy          0   TARGET   float   xy  
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_sampler s0, mode_default
dcl_resource_texture2darray (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_input_ps linear v1.xy
dcl_output o0.xy
dcl_temps 2
ftoi r0.xy, v0.xyxx
bfi r0.z, l(2), l(2), r0.y, l(0)
bfi r1.z, l(2), l(0), r0.x, r0.z
ishr r1.xy, r0.xyxx, l(2, 2, 0, 0)
mov r1.w, l(0)
ld_indexable(texture2darray)(float,float,float,float) r0.x, r1.xyzw, t0.xyzw
mov o0.x, r0.x
sample_indexable(texture2d)(float,float,float,float) r0.x, v1.xyxx, t1.xyzw, s0
mov o0.y, r0.x
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_ReinterleaveAO_PS_ENABLE_BLUR_1_DEPTH_LAYER_COUNT_1_D3D11[] =
{
     68,  88,  66,  67,  78, 212, 
     35, 117, 143, 115, 158, 122, 
    153, 136, 159, 204,  78, 252, 
    224, 241,   1,   0,   0,   0, 
     76,   2,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0, 184,   0, 
      0,   0,  73,  83,  71,  78, 
     80,   0,   0,   0,   2,   0, 
      0,   0,   8,   0,   0,   0, 
     56,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   3,   0,   0, 
     68,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   3,   3,   0,   0, 
     83,  86,  95,  80, 111, 115, 
    105, 116, 105, 111, 110,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171, 171, 171, 
     79,  83,  71,  78,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      3,  12,   0,   0,  83,  86, 
     95,  84,  65,  82,  71,  69, 
     84,   0, 171, 171,  83,  72, 
     69,  88, 140,   1,   0,   0, 
     80,   0,   0,   0,  99,   0, 
      0,   0, 106,   8,   0,   1, 
     90,   0,   0,   3,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     88,  64,   0,   4,   0, 112, 
     16,   0,   0,   0,   0,   0, 
     85,  85,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      1,   0,   0,   0,  85,  85, 
      0,   0, 100,  32,   0,   4, 
     50,  16,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     98,  16,   0,   3,  50,  16, 
     16,   0,   1,   0,   0,   0, 
    101,   0,   0,   3,  50,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   2,   0, 
      0,   0,  27,   0,   0,   5, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11,  66,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
    140,   0,   0,  11,  66,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,   0,  10, 
     50,   0,  16,   0,   1,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  45,   0,   0, 137, 
      2,   2,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  69,   0, 
      0, 139, 194,   0,   0, 128, 
     67,  85,  21,   0,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  16,  16,   0,   1,   0, 
      0,   0,  70, 126,  16,   0, 
      1,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   5,  34,  32, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  62,   0,   0,   1
};
#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xy  
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_TARGET                0   xy          0   TARGET   float   xy  
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_sampler s0, mode_default
dcl_resource_texture2darray (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_resource_texture2d (float,float,float,float) t2
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_input_ps linear v1.xy
dcl_output o0.xy
dcl_temps 2
ftoi r0.xy, v0.xyxx
bfi r0.z, l(2), l(2), r0.y, l(0)
bfi r1.z, l(2), l(0), r0.x, r0.z
ishr r1.xy, r0.xyxx, l(2, 2, 0, 0)
mov r1.w, l(0)
ld_indexable(texture2darray)(float,float,float,float) r0.x, r1.xyzw, t0.xyzw
mov o0.x, r0.x
sample_indexable(texture2d)(float,float,float,float) r0.x, v1.xyxx, t1.xyzw, s0
sample_indexable(texture2d)(float,float,float,float) r0.y, v1.xyxx, t2.yxzw, s0
min o0.y, r0.y, r0.x
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_ReinterleaveAO_PS_ENABLE_BLUR_1_DEPTH_LAYER_COUNT_2_D3D11[] =
{
     68,  88,  66,  67, 114, 202, 
     25, 200, 189,  22,  77,  38, 
    178, 204,  28, 164,  26,  54, 
     24,  75,   1,   0,   0,   0, 
    144,   2,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0, 184,   0, 
      0,   0,  73,  83,  71,  78, 
     80,   0,   0,   0,   2,   0, 
      0,   0,   8,   0,   0,   0, 
     56,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   3,   0,   0, 
     68,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   3,   3,   0,   0, 
     83,  86,  95,  80, 111, 115, 
    105, 116, 105, 111, 110,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171, 171, 171, 
     79,  83,  71,  78,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      3,  12,   0,   0,  83,  86, 
     95,  84,  65,  82,  71,  69, 
     84,   0, 171, 171,  83,  72, 
     69,  88, 208,   1,   0,   0, 
     80,   0,   0,   0, 116,   0, 
      0,   0, 106,   8,   0,   1, 
     90,   0,   0,   3,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     88,  64,   0,   4,   0, 112, 
     16,   0,   0,   0,   0,   0, 
     85,  85,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      1,   0,   0,   0,  85,  85, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   2,   0, 
      0,   0,  85,  85,   0,   0, 
    100,  32,   0,   4,  50,  16, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  98,  16, 
      0,   3,  50,  16,  16,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  50,  32,  16,   0, 
      0,   0,   0,   0, 104,   0, 
      0,   2,   2,   0,   0,   0, 
     27,   0,   0,   5,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  16,  16,   0,   0,   0, 
      0,   0, 140,   0,   0,  11, 
     66,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11,  66,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,   0,  10,  50,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      2,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     45,   0,   0, 137,   2,   2, 
      0, 128,  67,  85,  21,   0, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  69,   0,   0, 139, 
    194,   0,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  16, 
     16,   0,   1,   0,   0,   0, 
     70, 126,  16,   0,   1,   0, 
      0,   0,   0,  96,  16,   0, 
      0,   0,   0,   0,  69,   0, 
      0, 139, 194,   0,   0, 128, 
     67,  85,  21,   0,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  16,  16,   0,   1,   0, 
      0,   0,  22, 126,  16,   0, 
      2,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     51,   0,   0,   7,  34,  32, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1
};

namespace Generated
{
    void ReinterleaveAO_PS::Create(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::ENABLE_BLUR_0][ShaderPermutations::DEPTH_LAYER_COUNT_1].Create(Device, g_ReinterleaveAO_PS_ENABLE_BLUR_0_DEPTH_LAYER_COUNT_1_D3D11, sizeof(g_ReinterleaveAO_PS_ENABLE_BLUR_0_DEPTH_LAYER_COUNT_1_D3D11));
        m_Shader[ShaderPermutations::ENABLE_BLUR_0][ShaderPermutations::DEPTH_LAYER_COUNT_2].Create(Device, g_ReinterleaveAO_PS_ENABLE_BLUR_0_DEPTH_LAYER_COUNT_2_D3D11, sizeof(g_ReinterleaveAO_PS_ENABLE_BLUR_0_DEPTH_LAYER_COUNT_2_D3D11));
        m_Shader[ShaderPermutations::ENABLE_BLUR_1][ShaderPermutations::DEPTH_LAYER_COUNT_1].Create(Device, g_ReinterleaveAO_PS_ENABLE_BLUR_1_DEPTH_LAYER_COUNT_1_D3D11, sizeof(g_ReinterleaveAO_PS_ENABLE_BLUR_1_DEPTH_LAYER_COUNT_1_D3D11));
        m_Shader[ShaderPermutations::ENABLE_BLUR_1][ShaderPermutations::DEPTH_LAYER_COUNT_2].Create(Device, g_ReinterleaveAO_PS_ENABLE_BLUR_1_DEPTH_LAYER_COUNT_2_D3D11, sizeof(g_ReinterleaveAO_PS_ENABLE_BLUR_1_DEPTH_LAYER_COUNT_2_D3D11));
    }

    void ReinterleaveAO_PS::Release(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::ENABLE_BLUR_0][ShaderPermutations::DEPTH_LAYER_COUNT_1].Release(Device);
        m_Shader[ShaderPermutations::ENABLE_BLUR_0][ShaderPermutations::DEPTH_LAYER_COUNT_2].Release(Device);
        m_Shader[ShaderPermutations::ENABLE_BLUR_1][ShaderPermutations::DEPTH_LAYER_COUNT_1].Release(Device);
        m_Shader[ShaderPermutations::ENABLE_BLUR_1][ShaderPermutations::DEPTH_LAYER_COUNT_2].Release(Device);
    }
}
