
float3 g_vLightColor     : LightColor     = float3( 1.0f, 1.0f, 1.0f );        // Light color
float3 g_vLightDirection : LightDirection = float3( 0.0f, 1.0f, 0.0f );   // Light direction
float3 g_vCameraPosition : CameraPosition;

texture  g_normalTex : NormalTex;
texture  g_worldPosTex:WorldPosTex;

sampler g_normalTexSampler = sampler_state
{
    Texture = <g_normalTex>;    
    Filter = MIN_MAG_MIP_POINT;
	AddressU  = CLAMP;
	AddressV  = CLAMP;
};

sampler g_worldPosTexSampler = sampler_state
{
    Texture = <g_worldPosTex>;
    Filter = MIN_MAG_MIP_POINT;
	AddressU  = CLAMP;
	AddressV  = CLAMP;
};

struct VsInput
{
	float4 position: POSITION;
	float2 texCoord: TEXCOORD0;
};

struct PsInput
{
	float2 texCoord: TEXCOORD0;
};

struct PsOutput
{
	float4 diffuseAndSpecular: COLOR0;
};

VsInput mainVS(VsInput input)
{
	return input;
}

void mainPS(in PsInput input, out PsOutput output)
{
	float4 norDepth = tex2D(g_normalTexSampler, input.texCoord);
	float4 worldPos = tex2D(g_worldPosTexSampler, input.texCoord);
			
	float3 viewDir  = normalize(g_vCameraPosition - worldPos.xyz);
	float3 normal   = normalize(norDepth.xyz);
	float3 lightDir = normalize(g_vLightDirection.xyz);
	float3 reflDir  = reflect(lightDir, normal);
	float  diffuseFactor = saturate(dot(lightDir, normal));
	float  specularFactor= saturate(dot(-reflDir, viewDir));
	output.diffuseAndSpecular  = float4(diffuseFactor, specularFactor, 0,0);
}

technique technique0 {
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader = compile ps_2_0 mainPS();
	}
}
