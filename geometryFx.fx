//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float3 g_vMaterialAmbient : Ambient = float3( 0.2f, 0.2f, 0.2f );   // Material's ambient color
float3 g_vMaterialDiffuse : Diffuse = float3( 0.8f, 0.8f, 0.8f );   // Material's diffuse color
float3 g_vMaterialSpecular : Specular = float3( 1.0f, 1.0f, 1.0f );  // Material's specular color
float  g_fMaterialAlpha : Opacity = 1.0f;
int    g_nMaterialShininess : SpecularPower = 32;

float3 g_vLightColor : LightColor = float3( 1.0f, 1.0f, 1.0f );        // Light color
float3 g_vLightPosition : LightPosition = float3( 50.0f, 500.0f, 0.0f );   // Light position
float3 g_vCameraPosition : CameraPosition;
            
float    g_fTime : Time;            // App's time in seconds
float4x4 g_mView : View;          // World matrix
float4x4 g_mViewProjection : WorldViewProjection; // World * View * Projection matrix

texture  g_MeshTexture : Texture;   // Color texture for mesh
texture	 g_normalTex   : NormalTexture;
sampler MeshTextureSampler = sampler_state
{
    Texture = <g_MeshTexture>;    
    Filter  = MIN_MAG_MIP_LINEAR;
};
sampler g_normalSampler = sampler_state
{
	Texture = <g_normalTex>;    
    Filter  = MIN_MAG_MIP_LINEAR;
	AddressU= CLAMP;
	AddressV= CLAMP;
};

struct VsInput
{
	float4 position: POSITION;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
	float3 bitangent:BINORMAL;
	float2 texCoord: TEXCOORD0;
};

struct VsOutput
{
	float4 position: POSITION;
	float2 texCoord: TEXCOORD0;
	float3 normal:   TEXCOORD1;
	float3 tangent:  TEXCOORD2;
	float3 bitangent:  TEXCOORD3;
	float4 worldPos: TEXCOORD4;
};

struct PsInput
{
	float2 texCoord: TEXCOORD0;
	float3 normal:   TEXCOORD1;
	float3 tangent:  TEXCOORD2;
	float3 bitangent:  TEXCOORD3;
	float4 worldPos: TEXCOORD4;
};

struct PsOutput
{
	float4 normal:   COLOR0;
	float4 position: COLOR1;
};

//--------------------------------------------------------------------------------------
// Name: Projection
// Type: Vertex Shader Fragment
// Desc: Projection transform
//--------------------------------------------------------------------------------------
VsOutput Projection(VsInput input)
{
	VsOutput output;
	
    // Transform the position into world space for lighting, and projected space
    // for display
    float4 vPosWorld = mul( input.position, g_mView );
	output.worldPos = input.position;
    output.position = mul( input.position, g_mViewProjection );
    
    // Transform the normal into world space for lighting
	output.normal = normalize(input.normal);//normalize(mul( input.normal, (float3x3)g_mView ));
	output.tangent= normalize(input.tangent);//normalize(mul( input.tangent, (float3x3)g_mView ));
	output.bitangent= normalize(input.bitangent);//normalize(mul( input.bitangent, (float3x3)g_mView ));
    
    // Pass the texture coordinate
    output.texCoord = input.texCoord;  

	return output;
}



//--------------------------------------------------------------------------------------
// Name: Lighting
// Type: Pixel Shader
// Desc: Compute lighting and modulate the texture
//--------------------------------------------------------------------------------------
PsOutput GeometryInfo(PsInput input, uniform bool hasNormalMap)
{  
	PsOutput output;
    
	output.position = input.worldPos;
	float depth = distance(input.worldPos,g_vCameraPosition);
	
	float3 normal = input.normal.xyz;
	if(hasNormalMap)
	{
		float3x3 tanFrame;
		tanFrame[0] = normalize(input.tangent);
		tanFrame[1] = normalize(input.bitangent);
		tanFrame[2] = normalize(input.normal);
		float2 texCoord = input.texCoord;
		texCoord.y = 1 - texCoord.y;
		float3 normalTexVal = (tex2D(g_normalSampler, texCoord) - 0.5f) * 2.0f;
		normal = mul(normalTexVal, tanFrame);
	}
	normal = normalize(normal);
	output.normal  = float4(normal, depth);
	return output;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique GeoTechWithNormal
{
    pass P0
    {
        VertexShader = compile vs_2_0 Projection();    
        PixelShader = compile ps_2_0 GeometryInfo(true);    
    }
}

technique GeoTechWithoutNormal
{
    pass P0
    {
        VertexShader = compile vs_2_0 Projection();    
        PixelShader = compile ps_2_0 GeometryInfo(false);    
		
    }
}
