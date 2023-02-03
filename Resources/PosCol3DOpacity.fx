float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldmatrix : WorldMatrix;
float4x4 gViewInverseMatrix : ViewInverseMatrix;

Texture2D gDiffuseMap : DiffuseMap;


RasterizerState gRasterizerStateOpacity
{
	CullMode = none;
	FrontCounterClockwise = false;
};

BlendState gBlendStateOpacity
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilStateOpacity
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
};

SamplerState gSamStateOpacity : SampleState
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};


//------------------------------------------------
// Input/Output Struct
//------------------------------------------------
struct VertexShader_INPUT
{
	float3 Position : POSITION;
	float3 Tangent : TANGENT;
	float3 Normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PixelShader_INPUT
{
	float4 Position : SV_POSITION;
	float2 uv : TEXCOORD;
};

//------------------------------------------------
// VertexShader
//------------------------------------------------
PixelShader_INPUT VS(VertexShader_INPUT input)
{
    PixelShader_INPUT output = (PixelShader_INPUT)0;

    output.Position = float4(mul(float4(input.Position,1.f), gWorldViewProj));
    output.uv = input.uv;

    return output;
}

//------------------------------------------------
// Pixel Shader
//------------------------------------------------
float4 PS(PixelShader_INPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(gSamStateOpacity, input.uv);
}

technique11 DefaultTechniquePoint
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateOpacity);
		SetDepthStencilState(gDepthStencilStateOpacity, 0);
		SetBlendState(gBlendStateOpacity, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}