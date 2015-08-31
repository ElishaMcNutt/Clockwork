#pragma warning(disable:3571)

#ifdef COMPILEVS
    float3 GetAmbient(float zonePos)
    {
        return cAmbientStartColor + zonePos * cAmbientEndColor;
    }

    #ifdef NUMVERTEXLIGHTS
        float GetVertexLight(int index, float3 worldPos, float3 normal)
        {
            float3 lightDir = cVertexLights[index * 3 + 1].xyz;
            float3 lightPos = cVertexLights[index * 3 + 2].xyz;
            float invRange = cVertexLights[index * 3].w;
            float cutoff = cVertexLights[index * 3 + 1].w;
            float invCutoff = cVertexLights[index * 3 + 2].w;

            // Directional light
            if (invRange == 0.0)
            {
                float NdotL = max(dot(normal, lightDir), 0.0);
                return NdotL;
            }
            // Point/spot light
            else
            {
                float3 lightVec = (lightPos - worldPos) * invRange;
                float lightDist = length(lightVec);
                float3 localDir = lightVec / lightDist;
                float NdotL = max(dot(normal, localDir), 0.0);
                float atten = saturate(1.0 - lightDist * lightDist);
                float spotEffect = dot(localDir, lightDir);
                float spotAtten = saturate((spotEffect - cutoff) * invCutoff);
                return NdotL * atten * spotAtten;
            }
        }

        float GetVertexLightVolumetric(int index, float3 worldPos)
        {
            float3 lightDir = cVertexLights[index * 3 + 1].xyz;
            float3 lightPos = cVertexLights[index * 3 + 2].xyz;
            float invRange = cVertexLights[index * 3].w;
            float cutoff = cVertexLights[index * 3 + 1].w;
            float invCutoff = cVertexLights[index * 3 + 2].w;

            // Directional light
            if (invRange == 0.0)
            {
                return 1.0;
            }
            // Point/spot light
            else
            {
                float3 lightVec = (lightPos - worldPos) * invRange;
                float lightDist = length(lightVec);
                float3 localDir = lightVec / lightDist;
                float atten = saturate(1.0 - lightDist * lightDist);
                float spotEffect = dot(localDir, lightDir);
                float spotAtten = saturate((spotEffect - cutoff) * invCutoff);
                return atten * spotAtten;
            }
        }
    #endif

    #ifdef SHADOW

        #ifdef DIRLIGHT
            #define NUMCASCADES 4
        #else
            #define NUMCASCADES 1
        #endif

        void GetShadowPos(float4 projWorldPos, out float4 shadowPos[NUMCASCADES])
        {
            // Shadow projection: transform from world space to shadow space
            #if defined(DIRLIGHT)
                shadowPos[0] = mul(projWorldPos, cLightMatrices[0]);
                shadowPos[1] = mul(projWorldPos, cLightMatrices[1]);
                shadowPos[2] = mul(projWorldPos, cLightMatrices[2]);
                shadowPos[3] = mul(projWorldPos, cLightMatrices[3]);
            #elif defined(SPOTLIGHT)
                shadowPos[0] = mul(projWorldPos, cLightMatrices[1]);
            #else
                shadowPos[0] = float4(projWorldPos.xyz - cLightPos.xyz, 0.0);
            #endif
        }
    #endif
#endif

#ifdef COMPILEPS
    float GetDiffuse(float3 normal, float3 worldPos, out float3 lightDir)
    {
        #ifdef DIRLIGHT
            lightDir = cLightDirPS;
            return saturate(dot(normal, lightDir));
        #else
            float3 lightVec = (cLightPosPS.xyz - worldPos) * cLightPosPS.w;
            float lightDist = length(lightVec);
            lightDir = lightVec / lightDist;
            return saturate(dot(normal, lightDir)) * Sample2D(LightRampMap, float2(lightDist, 0.0)).r;
        #endif
    }

    float GetDiffuseVolumetric(float3 worldPos)
    {
        #ifdef DIRLIGHT
            return 1.0;
        #else
            float3 lightVec = (cLightPosPS.xyz - worldPos) * cLightPosPS.w;
            float lightDist = length(lightVec);
            return Sample2D(LightRampMap, float2(lightDist, 0.0)).r;
        #endif
    }

    float GetSpecular(float3 normal, float3 eyeVec, float3 lightDir, float specularPower)
    {
        float3 halfVec = normalize(normalize(eyeVec) + lightDir);
        return saturate(pow(dot(normal, halfVec), specularPower));
    }

    float GetIntensity(float3 color)
    {
        return dot(color, float3(0.299, 0.587, 0.114));
    }

    #ifdef SHADOW

        #ifdef DIRLIGHT
            #define NUMCASCADES 4
        #else
            #define NUMCASCADES 1
        #endif

        float GetShadow(float4 shadowPos)
        {
            #ifdef D3D11
                shadowPos.xyz /= shadowPos.w;
            #endif

            #ifndef LQSHADOW
                // Take four samples and average them
                // Note: in case of sampling a point light cube shadow, we optimize out the w divide as it has already been performed
                #if !defined(POINTLIGHT) && !defined(D3D11)
                    float2 offsets = cShadowMapInvSize * shadowPos.w;
                #else
                    float2 offsets = cShadowMapInvSize;
                #endif
                float4 shadowPos2 = float4(shadowPos.x + offsets.x, shadowPos.yzw);
                float4 shadowPos3 = float4(shadowPos.x, shadowPos.y + offsets.y, shadowPos.zw);
                float4 shadowPos4 = float4(shadowPos.xy + offsets.xy, shadowPos.zw);

                float4 inLight = float4(
                    SampleShadow(ShadowMap, shadowPos).r,
                    SampleShadow(ShadowMap, shadowPos2).r,
                    SampleShadow(ShadowMap, shadowPos3).r,
                    SampleShadow(ShadowMap, shadowPos4).r
                );
                #ifndef SHADOWCMP
                    return cShadowIntensity.y + dot(inLight, cShadowIntensity.x);
                #else
                    #ifndef POINTLIGHT
                        return cShadowIntensity.y + dot(inLight * shadowPos.w > shadowPos.z, cShadowIntensity.x);
                    #else
                        return cShadowIntensity.y + dot(inLight > shadowPos.z, cShadowIntensity.x);
                    #endif
                #endif
            #else
                // Take one sample
                float inLight = SampleShadow(ShadowMap, shadowPos).r;
                #ifndef SHADOWCMP
                    return cShadowIntensity.y + cShadowIntensity.x * inLight;
                #else
                    #ifndef POINTLIGHT
                        return cShadowIntensity.y + cShadowIntensity.x * (inLight * shadowPos.w > shadowPos.z);
                    #else
                        return cShadowIntensity.y + cShadowIntensity.x * (inLight > shadowPos.z);
                    #endif
                #endif
            #endif
        }

        #ifdef POINTLIGHT
            float GetPointShadow(float3 lightVec)
            {
                float3 axis = SampleCube(FaceSelectCubeMap, lightVec).rgb;
                float depth = abs(dot(lightVec, axis));

                // Expand the maximum component of the light vector to get full 0.0 - 1.0 UV range from the cube map,
                // and to avoid sampling across faces. Some GPU's filter across faces, while others do not, and in this
                // case filtering across faces is wrong
                const float factor = 1.0 / 256.0;
                lightVec += factor * axis * lightVec;

                // Read the 2D UV coordinates, adjust according to shadow map size and add face offset
                float4 indirectPos = SampleCube(IndirectionCubeMap, lightVec);
                indirectPos.xy *= cShadowCubeAdjust.xy;
                indirectPos.xy += float2(cShadowCubeAdjust.z + indirectPos.z * 0.5, cShadowCubeAdjust.w + indirectPos.w);

                float4 shadowPos = float4(indirectPos.xy, cShadowDepthFade.x + cShadowDepthFade.y / depth, 1.0);
                return GetShadow(shadowPos);
            }
        #endif

        #ifdef DIRLIGHT
            float GetDirShadowFade(float inLight, float depth)
            {
                return saturate(inLight + saturate((depth - cShadowDepthFade.z) * cShadowDepthFade.w));
            }

            float GetDirShadow(const float4 iShadowPos[NUMCASCADES], float depth)
            {
                float4 shadowPos;

                if (depth < cShadowSplits.x)
                    shadowPos = iShadowPos[0];
                else if (depth < cShadowSplits.y)
                    shadowPos = iShadowPos[1];
                else if (depth < cShadowSplits.z)
                    shadowPos = iShadowPos[2];
                else
                    shadowPos = iShadowPos[3];

                return GetDirShadowFade(GetShadow(shadowPos), depth);
            }

            float GetDirShadowDeferred(float4 projWorldPos, float depth)
            {
                float4 shadowPos;

                if (depth < cShadowSplits.x)
                    shadowPos = mul(projWorldPos, cLightMatricesPS[0]);
                else if (depth < cShadowSplits.y)
                    shadowPos = mul(projWorldPos, cLightMatricesPS[1]);
                else if (depth < cShadowSplits.z)
                    shadowPos = mul(projWorldPos, cLightMatricesPS[2]);
                else
                    shadowPos = mul(projWorldPos, cLightMatricesPS[3]);

                return GetDirShadowFade(GetShadow(shadowPos), depth);
            }
        #endif

        float GetShadow(float4 iShadowPos[NUMCASCADES], float depth)
        {
            #if defined(DIRLIGHT)
                return GetDirShadow(iShadowPos, depth);
            #elif defined(SPOTLIGHT)
                return GetShadow(iShadowPos[0]);
            #else
                return GetPointShadow(iShadowPos[0].xyz);
            #endif
        }

        float GetShadowDeferred(float4 projWorldPos, float depth)
        {
            #if defined(DIRLIGHT)
                return GetDirShadowDeferred(projWorldPos, depth);
            #elif defined(SPOTLIGHT)
                float4 shadowPos = mul(projWorldPos, cLightMatricesPS[1]);
                return GetShadow(shadowPos);
            #else
                float3 shadowPos = projWorldPos.xyz - cLightPosPS.xyz;
                return GetPointShadow(shadowPos);
            #endif
        }
    #endif

    #ifdef PBR

        float3 LinearToSRGB(float3 c)
        {
            return pow(c, 2.2);
        }

        float3 LinearFromSRGB(float3 c)
        {
            return pow(c, 1.0 / 2.2);
        }

    /// Diffuse factors

        /// Oren-Nayar diffuse factor
        ///     diffuseColor: input rgb
        ///     roughness: roughness of the surface
        ///     nDotV: dot prod of surface normal and view direction
        ///     nDotL: dot prod of surface normal and light direction
        ///     vDotH: dot prod of view direction and half-angle
        float3 OrenNayarDiffuse(in float3 diffuseColor, in float roughness, in float nDotV, in float nDotL, in float vDotH)
        {
            const float rough2 = roughness * roughness;
            
            const float vDotL = 2.0 * vDotH - 1.0;
            const float majorCtrl = 1.0 - 0.5 * rough2 / (rough2 + 0.33);
            const float cosRi = vDotL - nDotV * nDotL;
            const float minorCtrl = 0.45 * rough2 / (rough2 + 0.09) * cosRi * (cosRi >= 0 ? min(1, nDotL / nDotV) : nDotL);
            return diffuseColor / 3.141596 * ( nDotL * majorCtrl + minorCtrl);
        }

        /// Lambertian diffuse factor
        ///     diffuseColor: input rgb
        ///     roughness: roughness of the surface
        ///     nDotV: dot prod of surface normal and view direction
        ///     nDotL: dot prod of surface normal and light direction
        ///     vDotH: dot prod of view direction and half-angle
        float3 LambertianDiffuse(in float3 diffuseColor, in float roughness, in float nDotV, in float nDotL, in float vDotH)
        {
            return diffuseColor * nDotL;
        }
        
    /// Fresnel Terms

        /// Fresnel factor
        ///     specular: Specular color input
        ///     vDotH: dot product of view direction and half-angle
        float3 SchlickFresnel(in float3 specular, in float vDotH)
        {
            return specular + (float3(1.0, 1.0, 1.0) - specular) * pow(1.0 - vDotH, 5.0);
        }
        
        /// Fresnel factor, spherical gaussian in Schlick approximation; https://seblagarde.wordpress.com/2012/06/03/spherical-gaussien-approximation-for-blinn-phong-phong-and-fresnel/
        ///     specular: specular color of the surface
        ///     vDotH: dot product of view direction and half-angle
        float3 SchlickGaussianFresnel(in float3 specular, in float vDotH)
        {
            float sphericalGaussian = pow(2.0, (-5.55473 * vDotH - 6.98316) * vDotH);
            return specular + (float3(1.0, 1.0, 1.0) - specular) * sphericalGaussian;
        }
        
    // Visibility terms

        /// Smith GGX Visibility
        ///     nDotL: dot-prod of surface normal and light direction
        ///     nDotV: dot-prod of surface normal and view direction
        ///     roughness: surface roughness
        float SmithGGXVisibility(in float nDotL, in float nDotV, in float roughness)
        {
            float rough2 = roughness * roughness;
            float gSmithV = nDotV + sqrt(nDotV * (nDotV - nDotV * rough2) + rough2);
            float gSmithL = nDotL + sqrt(nDotL * (nDotL - nDotL * rough2) + rough2);
            return 1.0 / (gSmithV * gSmithL);
        }
        
        
        float SchlickG1(in float factor, in float rough2)
        {
            return 1.0 / (factor * (1.0 - rough2) + rough2);
        }
        
        /// Schlick approximation of Smith GGX
        ///     nDotL: dot product of surface normal and light direction
        ///     nDotV: dot product of surface normal and view direction
        ///     roughness: surface roughness
        float SchlickVisibility(float nDotL, float nDotV, float roughness)
        {
            const float rough2 = roughness * roughness;
            return (SchlickG1(nDotL, rough2) * SchlickG1(nDotV, rough2)) * 0.25; // divided by four
        }
        
    /// Normal Distributions
        
        /// Beckmann normal distribution
        ///     nDotH: dot-prod of surface normal and half-angle
        ///     roughness: surface roughness
        float BeckmannDistribution(in float nDotH, in float roughness)
        {
            const float rough2 = roughness * roughness;
            const float roughnessA = 1.0 / (4.0 * rough2 * pow(nDotH, 4));
            const float roughnessB = nDotH * nDotH - 1.0;
            const float roughnessC = rough2 * nDotH * nDotH;
            return roughnessA * exp(roughnessB / roughnessC);
        }
        
        /// Trowbridge-Reitz GGX normal distribution
        ///     nDotH: dot-prod of surface normal and half-angle
        ///     roughness: surface roughness
        float GGXDistribution(in float nDotH, in float roughness)
        {
            const float rough2 = roughness * roughness;
            const float tmp = roughness / max(1e-8, nDotH * nDotH * (rough2 - 1.0) + 1.0);
            return tmp * tmp * 0.3141596;
        }
        
    /// Functions
        
        #ifdef IBL      /// Image based lighting shading

            /// Epic's approximation, convenient outside of mobile as well - very tolerant of 'lazy' IBL such as unfiltered mips
            /// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
            ///     specColor: specular color of the fragment
            ///     roughness: surface roughness
            ///     nDotV: dot product of normal and view vectors
            float3 EnvBRDFApprox( half3 SpecularColor, half Roughness, half NoV )
            {
                // [ Lazarov 2013, "Getting More Physical in Call of Duty: Black Ops II" ]
                // Adaptation to fit our G term.
                const half4 c0 = { -1, -0.0275, -0.572, 0.022 };
            	const half4 c1 = { 1, 0.0425, 1.04, -0.04 };
            	half4 r = Roughness * c0 + c1;
            	half a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
            	half2 AB = half2( -1.04, 1.04 ) * a004 + r.zw;

                return SpecularColor * AB.x + AB.y;
            }

            /// Determine reflection vector based on surface roughness, rougher uses closer to the normal and smoother uses closer to the reflection vector
            ///     normal: surface normal
            ///     reflection: vector of reflection off of the surface
            ///     roughness: surface roughness
            float3 GetSpecularDominantDir(float3 normal, float3 reflection, float roughness) 
            { 
                float smoothness = saturate(1.0 - roughness); 
                float lerpFactor = smoothness * (sqrt(smoothness) + roughness); 
                return lerp(normal, reflection, lerpFactor);
            }
            
            /// Calculate IBL contributation
            ///     reflectVec: reflection vector for cube sampling
            ///     wsNormal: surface normal in word space
            ///     toCamera: normalized direction from surface point to camera
            ///     roughness: surface roughness
            ///     reflectionCubeColor: output of the sampled cubemap color
            float3 ImageBasedLighting(in float3 reflectVec, in float3 wsNormal, in float3 toCamera, in float3 specular, in float roughness, out float3 reflectionCubeColor)
            {    
                float ndv = abs(dot(wsNormal, toCamera) + 1e-5);
                
                reflectVec = GetSpecularDominantDir(wsNormal, -reflectVec, roughness);
                
                // Mip selection is something to tune to your desired results
                //const float mipSelect = 9;
                //const float mipSelect = 7 - 1 + log2(roughness); // Geilfus: https://github.com/simongeilfus/Cinder-Experiments
                float mipSelect = roughness  * 8;  // Lux-style
                
                reflectionCubeColor.rgb = SampleCubeLOD(ZoneCubeMap, float4(reflectVec, mipSelect)).rgb;
                float3 environmentSpecular = EnvBRDFApprox(specular, roughness, ndv);
                return environmentSpecular *  reflectionCubeColor;
            }

        #endif

    #endif

#endif