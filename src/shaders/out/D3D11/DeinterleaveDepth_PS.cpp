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
// SV_Target                2   x           2   TARGET   float   x   
// SV_Target                3   x           3   TARGET   float   x   
// SV_Target                4   x           4   TARGET   float   x   
// SV_Target                5   x           5   TARGET   float   x   
// SV_Target                6   x           6   TARGET   float   x   
// SV_Target                7   x           7   TARGET   float   x   
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[2], immediateIndexed
dcl_constantbuffer cb1[2], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.x
dcl_output o1.x
dcl_output o2.x
dcl_output o3.x
dcl_output o4.x
dcl_output o5.x
dcl_output o6.x
dcl_output o7.x
dcl_temps 2
round_ni r0.xy, v0.xyxx
mad r0.xy, r0.xyxx, l(4.000000, 4.000000, 0.000000, 0.000000), cb1[1].xyxx
add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
mul r0.xy, r0.xyxx, cb0[1].zwzz
gather4_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t0.xyzw, s0.x
gather4_aoffimmi_indexable(2,0,0)(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0.x
mov o0.x, r1.w
mov o1.x, r1.z
mov o2.x, r0.w
mov o3.x, r0.z
mov o4.x, r1.x
mov o5.x, r1.y
mov o6.x, r0.x
mov o7.x, r0.y
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_DeinterleaveDepth_PS_DEPTH_LAYER_COUNT_1_D3D11[] =
{
     68,  88,  66,  67,  68, 196, 
    145, 220,  76, 140, 243,  86, 
    140, 201,  64,  71,  88,  71, 
     88, 143,   1,   0,   0,   0, 
    184,   3,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0,  96,   1, 
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
     79,  83,  71,  78, 212,   0, 
      0,   0,   8,   0,   0,   0, 
      8,   0,   0,   0, 200,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   6,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   6,   0,   0,   0, 
      1,  14,   0,   0, 200,   0, 
      0,   0,   7,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   7,   0,   0,   0, 
      1,  14,   0,   0,  83,  86, 
     95,  84,  97, 114, 103, 101, 
    116,   0, 171, 171,  83,  72, 
     69,  88,  80,   2,   0,   0, 
     80,   0,   0,   0, 148,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      1,   0,   0,   0,   2,   0, 
      0,   0,  90,   0,   0,   3, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
    100,  32,   0,   4,  50,  16, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      0,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      2,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      3,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      4,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      5,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      6,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      7,   0,   0,   0, 104,   0, 
      0,   2,   2,   0,   0,   0, 
     65,   0,   0,   5,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  16,  16,   0,   0,   0, 
      0,   0,  50,   0,   0,  13, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  64, 
      0,   0, 128,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     70, 128,  32,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,  10,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  63,   0,   0, 
      0,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   8,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
    230, 138,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    109,   0,   0, 139, 194,   0, 
      0, 128,  67,  85,  21,   0, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
     10,  96,  16,   0,   0,   0, 
      0,   0, 109,   0,   0, 140, 
      1,   4,   0, 128, 194,   0, 
      0, 128,  67,  85,  21,   0, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
     10,  96,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   5,  18,  32,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   3,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5,  18,  32,  16,   0, 
      4,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   5,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5,  18,  32,  16,   0, 
      7,   0,   0,   0,  26,   0, 
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
// SV_Target                0   xy          0   TARGET   float   xy  
// SV_Target                1   xy          1   TARGET   float   xy  
// SV_Target                2   xy          2   TARGET   float   xy  
// SV_Target                3   xy          3   TARGET   float   xy  
// SV_Target                4   xy          4   TARGET   float   xy  
// SV_Target                5   xy          5   TARGET   float   xy  
// SV_Target                6   xy          6   TARGET   float   xy  
// SV_Target                7   xy          7   TARGET   float   xy  
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[2], immediateIndexed
dcl_constantbuffer cb1[2], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.xy
dcl_output o1.xy
dcl_output o2.xy
dcl_output o3.xy
dcl_output o4.xy
dcl_output o5.xy
dcl_output o6.xy
dcl_output o7.xy
dcl_temps 4
round_ni r0.xy, v0.xyxx
mad r0.xy, r0.xyxx, l(4.000000, 4.000000, 0.000000, 0.000000), cb1[1].xyxx
add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
mul r0.xy, r0.xyxx, cb0[1].zwzz
gather4_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t0.xyzw, s0.x
mov o0.x, r1.w
gather4_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.xyxx, t1.xyzw, s0.x
mov o0.y, r2.w
mov o1.x, r1.z
mov o1.y, r2.z
gather4_aoffimmi_indexable(2,0,0)(texture2d)(float,float,float,float) r3.xyzw, r0.xyxx, t0.xyzw, s0.x
gather4_aoffimmi_indexable(2,0,0)(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s0.x
mov o2.x, r3.w
mov o2.y, r0.w
mov o3.x, r3.z
mov o3.y, r0.z
mov o4.x, r1.x
mov o5.x, r1.y
mov o4.y, r2.x
mov o5.y, r2.y
mov o6.x, r3.x
mov o7.x, r3.y
mov o6.y, r0.x
mov o7.y, r0.y
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_DeinterleaveDepth_PS_DEPTH_LAYER_COUNT_2_D3D11[] =
{
     68,  88,  66,  67, 164,  62, 
    132, 211, 219, 147,  20, 107, 
    238,  85,  90, 127,  32, 107, 
     58, 235,   1,   0,   0,   0, 
    196,   4,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    132,   0,   0,   0,  96,   1, 
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
     79,  83,  71,  78, 212,   0, 
      0,   0,   8,   0,   0,   0, 
      8,   0,   0,   0, 200,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   6,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   6,   0,   0,   0, 
      3,  12,   0,   0, 200,   0, 
      0,   0,   7,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   7,   0,   0,   0, 
      3,  12,   0,   0,  83,  86, 
     95,  84,  97, 114, 103, 101, 
    116,   0, 171, 171,  83,  72, 
     69,  88,  92,   3,   0,   0, 
     80,   0,   0,   0, 215,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      1,   0,   0,   0,   2,   0, 
      0,   0,  90,   0,   0,   3, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
     88,  24,   0,   4,   0, 112, 
     16,   0,   1,   0,   0,   0, 
     85,  85,   0,   0, 100,  32, 
      0,   4,  50,  16,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   0,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   1,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   2,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   3,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   4,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   5,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   6,   0, 
      0,   0, 101,   0,   0,   3, 
     50,  32,  16,   0,   7,   0, 
      0,   0, 104,   0,   0,   2, 
      4,   0,   0,   0,  65,   0, 
      0,   5,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,  16, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,  13,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  64,   0,   0, 
    128,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,  70, 128, 
     32,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,  10,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,  63,   0,   0,   0,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  56,   0,   0,   8, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0, 230, 138, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 109,   0, 
      0, 139, 194,   0,   0, 128, 
     67,  85,  21,   0, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   0,  16,   0,   0,   0, 
      0,   0,  70, 126,  16,   0, 
      0,   0,   0,   0,  10,  96, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0, 109,   0,   0, 139, 
    194,   0,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   1,   0, 
      0,   0,  10,  96,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5,  34,  32,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
     34,  32,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0, 109,   0, 
      0, 140,   1,   4,   0, 128, 
    194,   0,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      3,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,  10,  96,  16,   0, 
      0,   0,   0,   0, 109,   0, 
      0, 140,   1,   4,   0, 128, 
    194,   0,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   1,   0, 
      0,   0,  10,  96,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5,  18,  32,  16,   0, 
      2,   0,   0,   0,  58,   0, 
     16,   0,   3,   0,   0,   0, 
     54,   0,   0,   5,  34,  32, 
     16,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   3,   0, 
      0,   0,  42,   0,  16,   0, 
      3,   0,   0,   0,  54,   0, 
      0,   5,  34,  32,  16,   0, 
      3,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   4,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   5,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   5,  34,  32,  16,   0, 
      4,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
     54,   0,   0,   5,  34,  32, 
     16,   0,   5,   0,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      3,   0,   0,   0,  54,   0, 
      0,   5,  18,  32,  16,   0, 
      7,   0,   0,   0,  26,   0, 
     16,   0,   3,   0,   0,   0, 
     54,   0,   0,   5,  34,  32, 
     16,   0,   6,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     34,  32,  16,   0,   7,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1
};

namespace Generated
{
    void DeinterleaveDepth_PS::Create(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::DEPTH_LAYER_COUNT_1].Create(Device, g_DeinterleaveDepth_PS_DEPTH_LAYER_COUNT_1_D3D11, sizeof(g_DeinterleaveDepth_PS_DEPTH_LAYER_COUNT_1_D3D11));
        m_Shader[ShaderPermutations::DEPTH_LAYER_COUNT_2].Create(Device, g_DeinterleaveDepth_PS_DEPTH_LAYER_COUNT_2_D3D11, sizeof(g_DeinterleaveDepth_PS_DEPTH_LAYER_COUNT_2_D3D11));
    }

    void DeinterleaveDepth_PS::Release(DevicePointer Device)
    {
        m_Shader[ShaderPermutations::DEPTH_LAYER_COUNT_1].Release(Device);
        m_Shader[ShaderPermutations::DEPTH_LAYER_COUNT_2].Release(Device);
    }
}
