struct VSIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};

struct VSOut
{
	float4 pos : SV_POSITION;
	float3 normal : TEXCOORD;
};

cbuffer Scene
{
	float4x4 worldViewProjMatrix;
	float4x4 worldMatrix;
};

VSOut VSMain(VSIn vsIn)
{
	VSOut output;
	output.pos = mul(float4(vsIn.pos.xyz, 1), worldViewProjMatrix);
	output.normal = mul(vsIn.normal.xyz, (float3x3)(worldMatrix));
	return output;
}

struct PSOutputDepthTextures
{
	float4 WorldNormal : SV_Target0;
};

PSOutputDepthTextures PSMain(VSOut vsOut)
{
	PSOutputDepthTextures OUT;

	float3 worldNormal = normalize(vsOut.normal);
	OUT.WorldNormal = float4(worldNormal.xyz, 1.f);

	return OUT;
}