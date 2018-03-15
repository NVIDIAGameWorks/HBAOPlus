cbuffer PerFrame : register(b0)
{
	row_major float4x4 MatView;
	row_major float4x4 MatViewProj;
};

cbuffer PerObject : register(b1)
{
	row_major float4x4 MatWorld;
};

struct VSInput
{
	float4 Position : POSITION;
};

struct PSInput 
{
	float4 Position : SV_Position;
	float3 ViewPosition : VIEWPOSITION;
};

void VSMain(in VSInput In, out PSInput Out)
{
	Out.Position = mul(In.Position, MatWorld);
	Out.ViewPosition = mul(Out.Position, (float3x3)MatView);
	Out.Position = mul(Out.Position, MatViewProj);
}

void PSMain(in PSInput In, out float4 Out : SV_Target)
{
	float3 Normal = -normalize(cross(ddx(In.ViewPosition), ddy(In.ViewPosition)));
	Out = float4(1.0, 1.0, 0.0, 1.0);
	Out = float4(Normal * 0.5 + 0.5, 1.0);
}