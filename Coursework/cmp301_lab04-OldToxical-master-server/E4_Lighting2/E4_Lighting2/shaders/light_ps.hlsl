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

    /*float3 lightVector = normalize(position[0].xyz - input.worldPosition);
    float3 dist = length(position[0].xyz - input.worldPosition);
    float attenuation = 1 / (0.5f + (0.125 * dist) + (0 * pow(dist, 2)));
    float4 diffuse2 = diffuse[0] * attenuation;
    float4 lightColour = ambient[0] + calculateLighting(lightVector, input.normal, diffuse2); // diffuse2 for attenuation
    float4 light1Final = calcSpecular(lightVector, input.normal, input.viewVector, specular[0], specularPower[0]) + lightColour;
    
    float3 lightVector2 = normalize(position[1].xyz - input.worldPosition);
    float3 dist2 = length(position[1].xyz - input.worldPosition);
    float attenuation2 = 1 / (0.5f + (0.125 * dist2) + (0 * pow(dist2, 2)));
    float4 diffuse2_2 = diffuse[1] * attenuation2;
    float4 lightColour2 = ambient[1] + calculateLighting(lightVector2, input.normal, diffuse2_2); // diffuse2 for attenuation
    float4 light2Final = calcSpecular(lightVector2, input.normal, input.viewVector, specular[0], specularPower[0]) + lightColour2;
    
    float3 lightVector3 = normalize(position[2].xyz - input.worldPosition);
    float3 dist3 = length(position[2].xyz - input.worldPosition);
    float attenuation3 = 1 / (0.5f + (0.125 * dist3) + (0 * pow(dist3, 2)));
    float4 diffuse2_3 = diffuse[2]; // * attenuation for attenuating the point light, for directinal it should be just the diffuse
    float4 lightColour3 = ambient[2] + calculateLighting(direction[2].xyz, input.normal, diffuse2_3); // diffuse2 for attenuation
    float4 light3Final = calcSpecular(direction[2].xyz, input.normal, input.viewVector, specular[0], specularPower[0]) + lightColour3;*/
}



