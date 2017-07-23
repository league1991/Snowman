float   g_brightThreshold :     BPThreshold  = 0.5;
float4  g_downSampleOffset[16]: SampleOffset;    	// 1-neighbour is in [0]-[3], 2-neighbour is in [4]-[15]

float4  g_offsetAndWeight[17]:   OffsetAndWeight;   // .x -> hoffset, .y -> hweight, .z.w is v component

float   g_fExposure:			ExposureFactor;
float	g_fWhite:				WhiteFactor = 1.5f;

texture g_srcTex: 				SourceTex;
texture g_luminanceTex:			LuminanceTex;
texture g_bloomTex:				BloomTex;
sampler g_srcSampler = sampler_state
{
    Texture   = <g_srcTex>;
	Filter    = MIN_MAG_MIP_LINEAR;
	AddressU  = CLAMP;
	AddressV  = CLAMP;
};
sampler g_luminanceSampler = sampler_state
{
    Texture   = <g_luminanceTex>;
	Filter    = MIN_MAG_MIP_LINEAR;
	AddressU  = CLAMP;
	AddressV  = CLAMP;
};
sampler g_bloomSampler = sampler_state
{
    Texture   = <g_bloomTex>;
	Filter    = MIN_MAG_MIP_LINEAR;
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

VsInput mainVS(VsInput input)
{
	return input;
}

float4  brightPassPS(PsInput input):COLOR0
{
	float4 average = {0,0,0,0};
	
	// downsample
	for(int i = 0; i < 4; ++i)
	{
		average += tex2D(g_srcSampler, input.texCoord + g_downSampleOffset[i]);
	}
	average *= 0.25f;
	
	// compute luminance
	float luminance = max(average.r, max(average.g, average.b));
	//float luminance = dot( average.rgb, float3( 0.299f, 0.587f, 0.114f ) );
	if(luminance < g_brightThreshold)
		average = float4(0,0,0,1);
	return average;
}

float4  luminancePs(PsInput input):COLOR0
{
	float  avgVal = 0;
	float  maxVal = 0;
	
	// downsample
	// such average method is very important!!!
	for(int i = 0; i < 4; ++i)
	{
		float4 val = tex2D(g_srcSampler, input.texCoord + g_downSampleOffset[i]);
		float luminance = dot( val.rgb, float3( 0.299f, 0.587f, 0.114f ) );
		avgVal += log( 1e-5 + luminance );
		maxVal = max(maxVal, luminance);
	}
	avgVal /= 4.0f;
	avgVal  = exp(avgVal);
    return float4(avgVal, maxVal,0,1);
}

float4  downSamplePS(PsInput input):COLOR0
{
	float4 average = { 0.0f, 0.0f, 0.0f, 0.0f };
    for( int i = 0; i < 16; i++ )
    {
        average += tex2D( g_srcSampler, input.texCoord + g_downSampleOffset[i]);
    }
    average *= ( 1.0f / 16.0f );

    return average;
}

float4  horiBlurPassPS(PsInput input):COLOR0
{
	float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };    
    for( int i = 0; i < 17; i++ )
    {
		float2 offsetWeight = g_offsetAndWeight[i].xy;
        color += (tex2D( g_srcSampler, input.texCoord + float2( offsetWeight.x, 0.0f ) ) * offsetWeight.y );
    }        
    return float4( color.rgb, 1.0f );
}

float4  vertBlurPassPS(PsInput input):COLOR0
{
	float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };    
    for( int i = 0; i < 17; i++ )
    {
		float2 offsetWeight = g_offsetAndWeight[i].zw;
        color += (tex2D( g_srcSampler, input.texCoord + float2( 0.0f, offsetWeight.x ) ) * offsetWeight.y );
    }        
    return float4( color.rgb, 1.0f );
}

float4  finalPS(PsInput input):COLOR0
{
	float4 c = tex2D(g_srcSampler, input.texCoord);
	float4 avgMaxLum = tex2D(g_luminanceSampler, float2(0.5, 0.5));
	float4 b = tex2D(g_bloomSampler, input.texCoord);
	
	float4 l = avgMaxLum.r;
	
	// actual color
	float4 final = c + 0.25f * b;
	//float Lp = (g_fExposure / l.r) * max( final.r, max( final.g, final.b ) );
	
	// maximum luminance
	//float LmSqr = avgMaxLum.g * avgMaxLum.g;
	//float toneScalar = Lp * (1.0f + Lp / LmSqr) / (1.0f + Lp);	
	//c = final * toneScalar;
	
	//
	c = final;	
	c.rgb *= g_fExposure / (l+0.001);
	c.rgb *= (1.0f + c/g_fWhite);
	c.rgb /= (1.0f + c);
		
	c.a = 1.f;
	return c;
}

technique brightPassTech {
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader  = compile ps_2_0 brightPassPS();
	}
}

technique luminanceTech{
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader  = compile ps_2_0 luminancePs();
	}
}

technique downSampleTech{
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader  = compile ps_2_0 downSamplePS();
	}
}

technique horiBlurTech{
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader  = compile ps_2_0 horiBlurPassPS();
	}
}

technique vertBlurTech{
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader  = compile ps_2_0 vertBlurPassPS();
	}
}

technique finalTech{
	pass p0 {
		CullMode = None;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader  = compile ps_2_0 finalPS();
	}
}
