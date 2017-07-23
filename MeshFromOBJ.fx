//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float3 g_vMaterialAmbient : Ambient = float3( 0.2f, 0.2f, 0.2f );   // Material's ambient color
float3 g_vMaterialDiffuse : Diffuse = float3( 0.8f, 0.8f, 0.8f );   // Material's diffuse color
float3 g_vMaterialSpecular : Specular = float3( 1.0f, 1.0f, 1.0f );  // Material's specular color
int    g_nMaterialShininess : SpecularPower = 32;

float3 g_vLightColor : LightColor = float3( 1.0f, 1.0f, 1.0f );        // Light color
float3 g_vLightPosition : LightPosition = float3( 50.0f, 500.0f, 0.0f );   // Light position
float3 g_vCameraPosition : CameraPosition;

float4x4 g_mWorldViewProjection : WorldViewProjection; // World * View * Projection matrix

float2 g_pixelOffset      : PixelOffset = float2(0,0);

texture g_MeshTexture   : Texture;   // Color texture for mesh
texture g_lightBufTex   : LightTex;
texture g_normalBufTex  : NormalTex;
texture g_objClrTex     : ObjColorTex;// object diffuse color(optional)
texture g_objSpecularTex: ObjSpecTex;// object specular value(optional)

sampler MeshTextureSampler = sampler_state
{
    Texture = <g_MeshTexture>;    
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};
sampler g_lightBufSampler = sampler_state
{
	Texture = <g_lightBufTex>;	
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};
sampler g_normalBufSampler = sampler_state
{
	Texture = <g_normalBufTex>;	
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};
sampler g_objClrSampler = sampler_state
{
	Texture = <g_objClrTex>;	
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};
sampler g_objSpecularSampler = sampler_state
{
	Texture = <g_objSpecularTex>;	
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

struct VsInput
{
	float4 position: POSITION;
	float3 normal:   NORMAL;
	float2 texCoord: TEXCOORD0;
};

struct VsOutput
{
	float4 position: POSITION;
	float2 texCoord: TEXCOORD0;
	float4 screenPos:TEXCOORD1;
};
struct PsInput
{
	float4 texCoord: TEXCOORD0;
	float2 screenPos:TEXCOORD1;
};
struct PsOutput
{
	float4 color:    COLOR0;
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
	float4 projPos = mul( input.position, g_mWorldViewProjection );
    output.position = projPos;
    
    // Pass the texture coordinate
    output.texCoord = input.texCoord;    
	output.screenPos = projPos;
	return output;
}



//--------------------------------------------------------------------------------------
// Name: Lighting
// Type: Pixel Shader
// Desc: Compute lighting and modulate the texture
//--------------------------------------------------------------------------------------
PsOutput Lighting(	VsOutput input, 
					uniform bool bDiffuse,
					uniform bool bSpecular)
{  
	PsOutput output;
    
	float2 screenPos = input.screenPos.xy / input.screenPos.w;
	screenPos = (screenPos*float2(1,-1) + float2(1,1)) * 0.5 + g_pixelOffset;
	
	// Compute the light vector
	float4 lightInfo     = tex2D(g_lightBufSampler,  screenPos);
	float  diffuseFactor = lightInfo.x;
	float  specularFactor= lightInfo.y;
	float4 normalBufVal  = tex2D(g_normalBufSampler,   screenPos);
    
	// fetch diffuse and specular
	float2 texCoord = input.texCoord.xy;
	texCoord.y = 1 - texCoord.y;
	float3 diffuseClr    = g_vMaterialDiffuse * diffuseFactor * g_vLightColor;
	if(bDiffuse)
		diffuseClr = tex2D(g_objClrSampler, texCoord);
	float3 specularClr   = g_vMaterialSpecular;
	if(bSpecular)
		specularClr = tex2D(g_objSpecularSampler, texCoord);
	
	output.color.rgb = diffuseClr + g_vLightColor * specularClr * pow(specularFactor, g_nMaterialShininess);
    output.color.rgb += g_vMaterialAmbient;
	output.color.a = 1;
	//output.color.xyz  = specularClr * pow(specularFactor, g_nMaterialShininess);//float4(normalBufVal.xyz ,1.0f);
	return output;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique DiffuseNoSpecular
{
    pass P0
    {
        VertexShader = compile vs_2_0 Projection();    
        PixelShader = compile ps_2_0 Lighting(true,false);    
    }
}

technique NoDiffuseNoSpecular
{
    pass P0
    {
        VertexShader = compile vs_2_0 Projection();    
        PixelShader = compile ps_2_0 Lighting(false,false);		
    }
}

technique DiffuseSpecular
{
    pass P0
    {
        VertexShader = compile vs_2_0 Projection();    
        PixelShader = compile ps_2_0 Lighting(true,true);  		
    }
}

technique NoDiffuseSpecular
{
    pass P0
    {
        VertexShader = compile vs_2_0 Projection();    
        PixelShader = compile ps_2_0 Lighting(false,true);		
    }
}