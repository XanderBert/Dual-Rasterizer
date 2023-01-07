float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false;
};

//------------------------------------------------
// Input/Output Struct
//------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 uv : TEXCOORD;

};

//------------------------------------------------
// Vertex Shader
//------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = float4(mul(float4(input.Position,1.f), gWorldViewProj));
	output.uv = input.uv;
	return output;
}

//------------------------------------------------
// Pixel Shader
//------------------------------------------------

	//------------------------------------------------
	// SamplerState
	//------------------------------------------------

//Anisotropic
SamplerState samPointAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPointAnisotropic,input.uv);
}

//Linear
SamplerState samPointLinear
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPointAnisotropic,input.uv);
}

//Point
SamplerState samPointPoint
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPointAnisotropic,input.uv);
}

//------------------------------------------------
// Technique
//------------------------------------------------

//Anisotropic
technique11 DefaultTechniqueAnisotropic
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
}

//Linear
technique11 DefaultTechniqueLinear
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
}

//Point
technique11 DefaultTechniquePoint
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}
}