// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 ambient[3];
    float4 diffuse[3];
    float4 position[3];
    float4 specular[3];
    float4 direction[3];
    float specularPower[4];
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 ldiffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(ldiffuse * intensity);
	return colour;
}

float4 calcSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
    // blinn-phong specular calculation
    float3 halfway = normalize(lightDirection + viewVector);
    float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
    return saturate(specularColour * specularIntensity);
}

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour = texture0.Sample(sampler0, input.tex);
    float4 finalLightColour = (0, 0, 0, 0);
    
    for (int i = 0; i < 3; i++)
    {
        float3 lightVector = normalize(position[i].xyz - input.worldPosition);
        float3 dist = length(position[i].xyz - input.worldPosition);
        float attenuation = 1 / (0.5f + (0.125 * dist) + (0 * pow(dist, 2)));
        float4 diffuse2 = (0, 0, 0, 0);
        float specularPower_ = 0;
        
        if (i != 2)
        {
            diffuse2 = diffuse[i] * attenuation;
            specularPower_ = specularPower[0];
        }
        else
        {
            diffuse2 = diffuse[i];
            specularPower_ = specularPower[2];
            lightVector = direction[2];
        }
        
        float4 lightColour = ambient[i] + calculateLighting(lightVector, input.normal, diffuse2); // diffuse2 for attenuation
        float4 lightFinal = calcSpecular(lightVector, input.normal, input.viewVector, specular[2], specularPower_) + lightColour;
        finalLightColour += lightFinal;
    }
    
    
    return finalLightColour * textureColour;
}



