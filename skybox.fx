// matrices
matrix matView  : VIEW;
matrix matProj  : PROJECTION;


//--------------------------------------------------------------------------------------
// Material Properties
//--------------------------------------------------------------------------------------

// Texture Parameter, annotation specifies default texture for EffectEdit
texture envTex <  string type = "CUBE"; string name = "skybox02.dds"; >;
float2 g_sphereUVOffset :SphereUVOffset = float2(0.f,0.f);
float g_factor:         EnvFactor = 1.f;

sampler linear_sampler = sampler_state
{
    Texture   = (envTex);
    Filter = MIN_MAG_MIP_LINEAR;
    ADDRESSU = Clamp;
    ADDRESSV = Clamp;
};

sampler sphereSampler = sampler_state
{
	Texture   = (envTex);
    Filter = MIN_MAG_MIP_LINEAR;
    ADDRESSU = Wrap;
    ADDRESSV = Clamp;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
void VS ( in  float3 v0   : POSITION,
          out float4 oPos : POSITION,
          out float3 oT0  : TEXCOORD0
		  )
{
    // Strip any translation off of the view matrix
    // Use only rotations & the projection matrix
    float4x4 matViewNoTrans =
    {
        matView[0],
        matView[1],
        matView[2],
        float4( 0.f, 0.f, 0.f, 1.f )
    };

    // Output the position
    oPos = mul( float4(v0,1.f), mul( matViewNoTrans, matProj ) );
    oT0 = v0;
		
	// project the cube to the far plane
	// oPos.z = oPos.w;
}




//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 cubePS( in  float3 t0      : TEXCOORD0): COLOR0
{		
	// The skybox texture is pre-lit, so simply output the texture color
   	float4 r0 = texCUBE( linear_sampler, t0 );
	r0.xyz *= g_factor;
	return r0;
}

float4 spherePS( in  float3 t0      : TEXCOORD0): COLOR0
{		
	// The skybox texture is pre-lit, so simply output the texture color
	float3 dir = normalize(t0);
	float angle = atan2(dir.x, dir.z);
	float  u = ((angle / 3.1415926f) + 1.0f) * 0.5f + g_sphereUVOffset.x;
	float  v = 1.0f - (dir.y + 1.0f) * 0.5f + g_sphereUVOffset.y;
	float4 clr = tex2D(sphereSampler, float2(u,v));	
	clr.xyz *= g_factor;
	return clr;
}
//--------------------------------------------------------------------------------------
// Default Technique
// Establishes Vertex and Pixel Shader
// Ensures base states are set to required values
// (Other techniques within the scene perturb these states)
//--------------------------------------------------------------------------------------
technique CubeTech
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 cubePS();
        
        ZEnable = TRUE;
        ZWriteEnable = FALSE;
        AlphaBlendEnable = FALSE;
        CullMode = None;
        AlphaTestEnable = FALSE;
    }
}

technique SphereTech
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 spherePS();
        
        ZEnable = TRUE;
        ZWriteEnable = FALSE;
        AlphaBlendEnable = FALSE;
        CullMode = None;
        AlphaTestEnable = FALSE;
    }
}

