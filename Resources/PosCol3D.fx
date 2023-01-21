float gPi = 3.1415;
float gLightIntensity = 7.0f;
float gShininess = 25.0f;
float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldmatrix : WorldMatrix;
float4x4 gViewInverseMatrix : ViewInverseMatrix;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap: SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;


RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = 1;
	DepthFunc = less;
	StencilEnable = false;
};

//------------------------------------------------
// BRDF Calculationc
//------------------------------------------------
float4 CalculateLambert(float kd, float4 cd)
{
	return cd * kd / gPi;
}

float CalculatePhong(float ks, float exp, float3 l, float3 v, float3 n)
{
	float3 reflectedLightVector = reflect(l,n);
	float reflectedViewDot = clamp(saturate(dot(reflectedLightVector, v)),0,1);
	float phong = ks * pow(reflectedViewDot, exp);

	return phong;
}

//------------------------------------------------
// Input/Output Struct
//------------------------------------------------

//Goes into VertexShader
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Tangent : TANGENT;
	float3 Normal : NORMAL;
	float2 uv : TEXCOORD;
};

//Goes into PixelShader
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : W_POSITION;
	float3 Tangent : TANGENT;
	float3 Normal : NORMAL;
	float2 uv : TEXCOORD;

};

//------------------------------------------------
// Vertex Shader
//------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = float4(mul(float4(input.Position,1.f), gWorldViewProj));
	output.WorldPosition = mul(float4(input.Position, 1.0f), gWorldmatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldmatrix);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldmatrix);
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
//ERROR
	float3 binormal = cross(input.Normal, input.Tangent);
	float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.0f), float4(binormal, 0.0f), float4(input.Normal, 0.0), float4(0.0f, 0.0f, 0.0f, 1.0f));
	float3 currentNormalMap = 2.0f * gNormalMap.Sample(samPointAnisotropic, input.uv).rgb - float3(1.0f, 1.0f, 1.0f);
	float3 normal = mul(float4(currentNormalMap, 0.0f), tangentSpaceAxis);

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

	float observedArea = saturate(dot(normal, -gLightDirection));

	float4 lambert = CalculateLambert(1.0f, gDiffuseMap.Sample(samPointAnisotropic, input.uv));

	float specularExp = gShininess * gGlossinessMap.Sample(samPointAnisotropic, input.uv).r;
	float4 specular = gSpecularMap.Sample(samPointAnisotropic, input.uv) * CalculatePhong(1.0f, specularExp, -gLightDirection, viewDirection, input.Normal);

	return (gLightIntensity * lambert + specular) * observedArea;
}

//Linear
SamplerState samPointLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPointLinear,input.uv);
}

//Point
SamplerState samPointPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPointPoint,input.uv);
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
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
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