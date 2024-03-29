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
// SV_TARGET                0   x           0   TARGET   float   x   
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[8], immediateIndexed
dcl_resource_texture2d (float,float,float,float) t0
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.x
dcl_temps 1
add r0.xy, v0.xyxx, cb0[7].xyxx
ftoi r0.xy, r0.xyxx
mov r0.zw, l(0,0,0,0)
ld_indexable(texture2d)(float,float,float,float) r0.x, r0.xyzw, t0.xyzw
mad_sat r0.x, cb0[6].z, r0.x, cb0[6].w
mad r0.x, r0.x, cb0[6].x, cb0[6].y
div o0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_LinearizeDepth_PS_RESOLVE_DEPTH_0_DEPTH_LAYER_COUNT_1_D3D11[] =
{
     68,  88,  66,  67, 143, 107, 
     98, 163,  29, 194, 143,  73, 
    101,  95, 229,  36, 236, 183, 
    118,  34,   1,   0,   0,   0, 
     12,   2,   0,   0,   3,   0, 
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
      1,  14,   0,   0,  83,  86, 
     95,  84,  65,  82,  71,  69, 
     84,   0, 171, 171,  83,  72, 
     69,  88,  76,   1,   0,   0, 
     80,   0,   0,   0,  83,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0, 100,  32,   0,   4, 
     50,  16,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3,  18,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   1,   0, 
      0,   0,   0,   0,   0,   8, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      0,   0,   0,   0,  70, 128, 
     32,   0,   0,   0,   0,   0, 
      7,   0,   0,   0,  27,   0, 
      0,   5,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 194,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  45,   0,   0, 137, 
    194,   0,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,  50,  32,   0,  11, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  50,   0, 
      0,  11,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     14,   0,   0,  10,  18,  32, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,  10,   0,  16,   0, 
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
// SV_Target                0   x           0   TARGET   float   x   
// SV_Target                1   x           1   TARGET   float   x   
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[8], immediateIndexed
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.x
dcl_output o1.x
dcl_temps 2
add r0.xy, v0.xyxx, cb0[7].xyxx
ftoi r0.xy, r0.xyxx
mov r0.zw, l(0,0,0,0)
ld_indexable(texture2d)(float,float,float,float) r1.x, r0.xyww, t0.xyzw
ld_indexable(texture2d)(float,float,float,float) r0.x, r0.xyzw, t1.xyzw
mad_sat r0.x, cb0[6].z, r0.x, cb0[6].w
mad r0.x, r0.x, cb0[6].x, cb0[6].y
div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
mad_sat r0.y, cb0[6].z, r1.x, cb0[6].w
mad r0.y, r0.y, cb0[6].x, cb0[6].y
div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
min o0.x, r0.x, r0.y
max o1.x, r0.x, r0.y
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_LinearizeDepth_PS_RESOLVE_DEPTH_0_DEPTH_LAYER_COUNT_2_D3D11[] =
{
     68,  88,  66,  67, 234, 208, 
    159, 244, 198, 183,  43,  95, 
     13, 175, 226, 221,  96, 188, 
    220, 229,   1,   0,   0,   0, 
     28,   3,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0, 208,   0, 
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
     79,  83,  71,  78,  68,   0, 
      0,   0,   2,   0,   0,   0, 
      8,   0,   0,   0,  56,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  14,   0,   0,  56,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      1,  14,   0,   0,  83,  86, 
     95,  84,  97, 114, 103, 101, 
    116,   0, 171, 171,  83,  72, 
     69,  88,  68,   2,   0,   0, 
     80,   0,   0,   0, 145,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   1,   0, 
      0,   0,  85,  85,   0,   0, 
    100,  32,   0,   4,  50,  16, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      0,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   2,   2,   0,   0,   0, 
      0,   0,   0,   8,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  16,  16,   0,   0,   0, 
      0,   0,  70, 128,  32,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  27,   0,   0,   5, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 194,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     45,   0,   0, 137, 194,   0, 
      0, 128,  67,  85,  21,   0, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  70,  15,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
     45,   0,   0, 137, 194,   0, 
      0, 128,  67,  85,  21,   0, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   1,   0,   0,   0, 
     50,  32,   0,  11,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     42, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  50,   0,   0,  11, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  26, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  14,   0, 
      0,  10,  18,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  50,  32,   0,  11, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  50,   0, 
      0,  11,  34,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     14,   0,   0,  10,  34,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,  26,   0,  16,   0, 
      0,   0,   0,   0,  51,   0, 
      0,   7,  18,  32,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  52,   0,   0,   7, 
     18,  32,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     62,   0,   0,   1
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
// SV_TARGET                0   x           0   TARGET   float   x   
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[8], immediateIndexed
dcl_resource_texture2dms(0) (float,float,float,float) t0
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.x
dcl_temps 1
add r0.xy, v0.xyxx, cb0[7].xyxx
ftoi r0.xy, r0.xyxx
mov r0.zw, l(0,0,0,0)
ldms_indexable(texture2dms)(float,float,float,float) r0.x, r0.xyzw, t0.xyzw, l(0)
mad_sat r0.x, cb0[6].z, r0.x, cb0[6].w
mad r0.x, r0.x, cb0[6].x, cb0[6].y
div o0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_LinearizeDepth_PS_RESOLVE_DEPTH_1_DEPTH_LAYER_COUNT_1_D3D11[] =
{
     68,  88,  66,  67,  89, 242, 
    176, 163, 192,   9,  27, 137, 
     99, 230,  79,  67, 181, 241, 
    226,  37,   1,   0,   0,   0, 
     20,   2,   0,   0,   3,   0, 
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
      1,  14,   0,   0,  83,  86, 
     95,  84,  65,  82,  71,  69, 
     84,   0, 171, 171,  83,  72, 
     69,  88,  84,   1,   0,   0, 
     80,   0,   0,   0,  85,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  88,  32, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0, 100,  32,   0,   4, 
     50,  16,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3,  18,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   1,   0, 
      0,   0,   0,   0,   0,   8, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,  16,  16,   0, 
      0,   0,   0,   0,  70, 128, 
     32,   0,   0,   0,   0,   0, 
      7,   0,   0,   0,  27,   0, 
      0,   5,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 194,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  46,   0,   0, 139, 
      2,   1,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  50,  32, 
      0,  11,  18,   0,  16,   0, 
      0,   0,   0,   0,  42, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     50,   0,   0,  11,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  26, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  14,   0,   0,  10, 
     18,  32,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     62,   0,   0,   1
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
// SV_Target                0   x           0   TARGET   float   x   
// SV_Target                1   x           1   TARGET   float   x   
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[8], immediateIndexed
dcl_resource_texture2dms(0) (float,float,float,float) t0
dcl_resource_texture2dms(0) (float,float,float,float) t1
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.x
dcl_output o1.x
dcl_temps 2
add r0.xy, v0.xyxx, cb0[7].xyxx
ftoi r0.xy, r0.xyxx
mov r0.zw, l(0,0,0,0)
ldms_indexable(texture2dms)(float,float,float,float) r1.x, r0.xyww, t0.xyzw, l(0)
ldms_indexable(texture2dms)(float,float,float,float) r0.x, r0.xyzw, t1.xyzw, l(0)
mad_sat r0.x, cb0[6].z, r0.x, cb0[6].w
mad r0.x, r0.x, cb0[6].x, cb0[6].y
div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
mad_sat r0.y, cb0[6].z, r1.x, cb0[6].w
mad r0.y, r0.y, cb0[6].x, cb0[6].y
div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
min o0.x, r0.x, r0.y
max o1.x, r0.x, r0.y
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_LinearizeDepth_PS_RESOLVE_DEPTH_1_DEPTH_LAYER_COUNT_2_D3D11[] =
{
     68,  88,  66,  67, 169, 223, 
    209, 103,  10, 214, 143,  58, 
    228,  54, 105,  11, 108, 117, 
    141, 225,   1,   0,   0,   0, 
     44,   3,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0, 208,   0, 
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
     79,  83,  71,  78,  68,   0, 
      0,   0,   2,   0,   0,   0, 
      8,   0,   0,   0,  56,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  14,   0,   0,  56,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      1,  14,   0,   0,  83,  86, 
     95,  84,  97, 114, 103, 101, 
    116,   0, 171, 171,  83,  72, 
     69,  88,  84,   2,   0,   0, 
     80,   0,   0,   0, 149,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  88,  32, 
      0,   4,   0, 112,  16,   0, 
      0,   0,   0,   0,  85,  85, 
      0,   0,  88,  32,   0,   4, 
      0, 112,  16,   0,   1,   0, 
      0,   0,  85,  85,   0,   0, 
    100,  32,   0,   4,  50,  16, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      0,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   2,   2,   0,   0,   0, 
      0,   0,   0,   8,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  16,  16,   0,   0,   0, 
      0,   0,  70, 128,  32,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  27,   0,   0,   5, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 194,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     46,   0,   0, 139,   2,   1, 
      0, 128,  67,  85,  21,   0, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  70,  15,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  46,   0,   0, 139, 
      2,   1,   0, 128,  67,  85, 
     21,   0,  18,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  50,  32, 
      0,  11,  18,   0,  16,   0, 
      0,   0,   0,   0,  42, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     50,   0,   0,  11,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  26, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  14,   0,   0,  10, 
     18,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     50,  32,   0,  11,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     42, 128,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  50,   0,   0,  11, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  26, 128, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  14,   0, 
      0,  10,  34,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  51,   0,   0,   7, 
     18,  32,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     52,   0,   0,   7,  18,  32, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1
};

namespace Generated
{
    void LinearizeDepth_PS::Create(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_0][ShaderPermutations::DEPTH_LAYER_COUNT_1].Create(Device, g_LinearizeDepth_PS_RESOLVE_DEPTH_0_DEPTH_LAYER_COUNT_1_D3D11, sizeof(g_LinearizeDepth_PS_RESOLVE_DEPTH_0_DEPTH_LAYER_COUNT_1_D3D11));
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_0][ShaderPermutations::DEPTH_LAYER_COUNT_2].Create(Device, g_LinearizeDepth_PS_RESOLVE_DEPTH_0_DEPTH_LAYER_COUNT_2_D3D11, sizeof(g_LinearizeDepth_PS_RESOLVE_DEPTH_0_DEPTH_LAYER_COUNT_2_D3D11));
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_1][ShaderPermutations::DEPTH_LAYER_COUNT_1].Create(Device, g_LinearizeDepth_PS_RESOLVE_DEPTH_1_DEPTH_LAYER_COUNT_1_D3D11, sizeof(g_LinearizeDepth_PS_RESOLVE_DEPTH_1_DEPTH_LAYER_COUNT_1_D3D11));
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_1][ShaderPermutations::DEPTH_LAYER_COUNT_2].Create(Device, g_LinearizeDepth_PS_RESOLVE_DEPTH_1_DEPTH_LAYER_COUNT_2_D3D11, sizeof(g_LinearizeDepth_PS_RESOLVE_DEPTH_1_DEPTH_LAYER_COUNT_2_D3D11));
    }

    void LinearizeDepth_PS::Release(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_0][ShaderPermutations::DEPTH_LAYER_COUNT_1].Release(Device);
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_0][ShaderPermutations::DEPTH_LAYER_COUNT_2].Release(Device);
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_1][ShaderPermutations::DEPTH_LAYER_COUNT_1].Release(Device);
        m_Shader[ShaderPermutations::RESOLVE_DEPTH_1][ShaderPermutations::DEPTH_LAYER_COUNT_2].Release(Device);
    }
}
