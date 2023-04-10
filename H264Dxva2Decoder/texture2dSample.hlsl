Texture2D  luminanceChannel   : register(t0);
Texture2D chrominanceChannel : register(t1);
SamplerState  defaultSampler     : register(s0);
struct VS_Input
{
    float4 pos : POSITION;
    float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};
PS_Input VS_Main(VS_Input vertex)
{
    PS_Input vsOut = (PS_Input)0;
    vsOut.pos = vertex.pos;
    vsOut.tex0 = vertex.tex0;
    return vsOut;
}
static const float3x3 YUVtoRGBCoeffMatrix =
{
	1.164383f,  1.164383f, 1.164383f,
	0.000000f, -0.391762f, 2.017232f,
	1.596027f, -0.812968f, 0.000000f
};
float3 ConvertYUVtoRGB(float3 yuv)
{
	// Derived from https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
	// Section: Converting 8-bit YUV to RGB888

	// These values are calculated from (16 / 255) and (128 / 255)
	yuv -= float3(0.062745f, 0.501960f, 0.501960f);
	yuv = mul(yuv, YUVtoRGBCoeffMatrix);

	return saturate(yuv);
}

float4 PS_Main(PS_Input input) : SV_TARGET
{
	float y = luminanceChannel.Sample(defaultSampler, input.tex0);
	float2 uv = chrominanceChannel.Sample(defaultSampler, input.tex0);
    return float4(ConvertYUVtoRGB(float3(y, uv)), 1.f);
}