//#version 430
//precision highp float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// ---------------------
// HIT FUNCTIONS
// ---------------------

//SPHERE
bool HitSphere(const in THitable aSphere, in TRay aRay, in float aMin, in float aMax, inout THitRecord aRecord )
{
    vec3 oc = aRay.mOrigin - aSphere.mParametersA.xyz;
    float a = dot(aRay.mDirection, aRay.mDirection);
    float b = dot(oc, aRay.mDirection);
    float c = dot(oc, oc) - aSphere.mParametersA.w * aSphere.mParametersA.w;
    float discriminant = b*b - a*c;
    
    if(discriminant > 0)
    {
        {
            float lTemp = (-b - sqrt(b*b-a*c)) / a;
            if(lTemp < aMax && lTemp > aMin)
            {
                aRecord.mDistance = lTemp;
                aRecord.mPoint = RayPointAt(aRay, aRecord.mDistance);
                aRecord.mNormal = (aRecord.mPoint - aSphere.mParametersA.xyz) / aSphere.mParametersA.w;
                aRecord.mMaterialId = aSphere.mMaterialId;
                return true;
            }
        }
        {
            float lTemp = (-b + sqrt(b*b-a*c)) / a;
            if(lTemp < aMax && lTemp > aMin)
            {
                aRecord.mDistance = lTemp;
                aRecord.mPoint = RayPointAt(aRay, aRecord.mDistance);
                aRecord.mNormal = (aRecord.mPoint - aSphere.mParametersA.xyz) / aSphere.mParametersA.w;
                aRecord.mMaterialId = aSphere.mMaterialId;
                return true;
            }
        }
    }
    return false;
}

// PLANE
bool HitPlane(const in THitable aPlane, in TRay aRay, in float aMin, in float aMax, inout THitRecord aRecord )
{
    const float x0 = aPlane.mParametersA.x;
    const float x1 = aPlane.mParametersA.y;
    const float y0 = aPlane.mParametersA.z;
    const float y1 = aPlane.mParametersA.w;
    const float k = aPlane.mParametersB.x;
    const float t = (k - aRay.mOrigin.z) / aRay.mDirection.z;
    if(t < aMin || t > aMax)
    {
        return false;
    }

    float x = aRay.mOrigin.x + t*aRay.mDirection.x;
    float y = aRay.mOrigin.y + t*aRay.mDirection.y;

    if(x < x0 || x > x1 || y < y0 || y > y1)
    {
        return false;
    }

    aRecord.mUV.x = (x-x0)/(x1-x0);
    aRecord.mUV.y = (y-y0)/(y1-y0);
    aRecord.mDistance = t;
    aRecord.mMaterialId = aPlane.mMaterialId;
    aRecord.mPoint = RayPointAt(aRay, aRecord.mDistance);
    aRecord.mNormal = vec3(0, 0, aPlane.mParametersB.w);

    return true;
}

// OBJECT SWITCH
bool HitObject(in uint aObjectIndex, in TRay aRay, in float aMin, in float aMax, inout THitRecord aRecord )
{
    switch(gHitablesList[aObjectIndex].mType)
    {
    case HITABLE_SPHERE:
    {
        return HitSphere(gHitablesList[aObjectIndex], aRay, aMin, aMax, aRecord);
    }
    case HITABLE_PLANE:
    {
        return HitPlane(gHitablesList[aObjectIndex], aRay, aMin, aMax, aRecord);
    }
    case HITABLE_RM_MAP:
    {
        return HitRayMarching(aRay, aMin, aMax, aRecord);
    }
    default:
        break;
    }

    return false;
}

// WORLD ITERATOR
bool HitWorld(in TRay aRay, in float aMin, in float aMax, inout THitRecord aRecord )
{
    THitRecord lTempRecord;
    lTempRecord.mUV = aRecord.mUV;
    bool lHitAnything = false;
    float lClosest = FLT_MAX;
    for(int i = 0; i < MAX_HITABLES; ++i)
    {
        if(HitObject(i, aRay, aMin, lClosest, lTempRecord))
        {
            lHitAnything = true;
            lClosest = lTempRecord.mDistance;
            aRecord = lTempRecord;
        }
    }
    return lHitAnything;
}

// ---------------------
// COLOR FUNCTIONS
// ---------------------

// MATERIAL
void ProcessMaterial(const in THitRecord aRecord, out TMaterial aOutMaterial)
{
    switch(aRecord.mMaterialId)
    {
        case MAT_CORNELL_BOX:
        {
            //aOutMaterial = gMaterialsList[1]; 
            aOutMaterial = MakeSolidMat(vec4(1., 1., 1., 0.0), 1.0, 0.0);

            if(aRecord.mNormal.x > 0.9)
            {
                aOutMaterial = MakeSolidMat(vec4(0.12, 0.45, 0.15, 0.0), 1.0, 0.0);
            }
            else if(aRecord.mNormal.x < -0.9)
            {
                aOutMaterial = MakeSolidMat(vec4(0.65, 0.05, 0.05, 0.0), 1.0, 0.0);
            }
            else if(aRecord.mNormal.y < -0.9)
            {
                //aOutMaterial = MakeSolidMat(vec4(1.0, 1.0, 1.0, 1.1), 1.0, 0.0);
            }

            break;        
        }
        default:
        {
            aOutMaterial = (aRecord.mMaterialId < MAX_MATERIALS) ? gMaterialsList[aRecord.mMaterialId] : MakeSolidMat(vec4(1., 0.5, 1.0, 2.0), 1.0, 0.0);   
            break;
        }
    }
}

// BRDF - Schlick
float Schlick(float aCosine, float aRefraction)
{
    float lR0 = (1.0f - aRefraction) / (1.0f + aRefraction);
    lR0 = lR0 * lR0;
    return lR0 + (1.0f - lR0) * pow((1.0f - aCosine), 5.0f);
}

// BRDF
bool BRDF(const in TRay aRay, const in THitRecord aRecord, out vec4 aOutAttenuation, out TRay aOutScatteredRay, out float aOutPDF)
{
    TMaterial lMaterial;// = gMaterialsList[aRecord.mMaterialId];
    ProcessMaterial(aRecord, lMaterial);
   
    if(lMaterial.mMetalness > 0.0f)
    {
        //metal
        vec3 lReflected = reflect(aRay.mDirection, aRecord.mNormal);
        aOutScatteredRay = TRay(aRecord.mPoint, normalize(lReflected + lMaterial.mRoughness * RandomPointInSphere(aRecord.mUV)));
        aOutAttenuation = lMaterial.mAlbedo;
        return (dot(aOutScatteredRay.mDirection, aRecord.mNormal) > 0.f);
    }
    else if(lMaterial.mRefraction > 0.0f )
    {
        //translucent objects
        const vec3 lReflected = reflect(normalize(aRay.mDirection), aRecord.mNormal);
        aOutAttenuation = lMaterial.mAlbedo;
        vec3 lOutwardNormal;
        vec3 lRefracted;
        float lNiOverNt;
        float lReflectProb;
        float lCosine;

        if(dot(aRay.mDirection, aRecord.mNormal) > 0)
        {
            lOutwardNormal = -aRecord.mNormal;
            lNiOverNt = lMaterial.mRefraction;
            lCosine = lMaterial.mRefraction * dot(aRay.mDirection, aRecord.mNormal) / length(aRay.mDirection);
        }
        else
        {
            lOutwardNormal = aRecord.mNormal;
            lNiOverNt = 1.0f / lMaterial.mRefraction;
            lCosine = -dot(aRay.mDirection, aRecord.mNormal) / length(aRay.mDirection);
        }

        lRefracted = refract(aRay.mDirection, lOutwardNormal, lNiOverNt );
        if(length(lRefracted) > 0.0)
        {
            //aOutScatteredRay = TRay(aRecord.mPoint, lRefracted);
            lReflectProb = Schlick(lCosine, lMaterial.mRefraction);
        }
        else
        {
            //aOutScatteredRay = TRay(aRecord.mPoint, lReflected);
            lReflectProb = 1.0f;
        }

        if(NRand(aRecord.mUV) < lReflectProb)
        {
            const vec3 lTarget = lReflected + lMaterial.mRoughness * RandomPointInSphere(aRecord.mUV);
            aOutScatteredRay = TRay(aRecord.mPoint, normalize(lTarget));
        }
        else
        {
            const vec3 lTarget = lRefracted + lMaterial.mRoughness * RandomPointInSphere(aRecord.mUV);
            aOutScatteredRay = TRay(aRecord.mPoint, normalize(lTarget));
        }

        return true;
    }
    else if(lMaterial.mRoughness >= 1.0f)
    {
        //perfect lambertian
        /*TOrthoNormalBases lONB;
        ONB_MakeFromNormal(lONB, aRecord.mNormal);
        vec3 lDirection = ONB_Local(lONB, RandomCosineDirection(aRecord.mUV) );
        aOutScatteredRay = TRay(aRecord.mPoint, normalize(lDirection));
        aOutAttenuation = lMaterial.mAlbedo;
        aOutPDF = dot(lONB.mAxisW, aOutScatteredRay.mDirection) / M_PI;*/
        
        const vec3 lTarget = aRecord.mPoint + aRecord.mNormal + lMaterial.mRoughness * RandomPointInSphere(aRecord.mUV);
        aOutScatteredRay = TRay(aRecord.mPoint, normalize(lTarget - aRecord.mPoint));
        aOutAttenuation = lMaterial.mAlbedo;
        aOutPDF = dot(aRecord.mNormal, aOutScatteredRay.mDirection) / M_PI;
        return true;
    }

    return false;
}

float ScatteringPDF(const in TRay aRay, const in THitRecord aRecord, const in TRay aScatteredRay)
{
    float lCosine = dot(aRecord.mNormal, normalize(aScatteredRay.mDirection));
    if(lCosine < 0.) lCosine = 0.;
    return lCosine / M_PI;
}

// ---------------------
// RAYTRACER FUNCTIONS
// ---------------------

struct TComputeColorStack
{
    vec4 mColor;
    vec4 mEmissive;
    float mScatteringPDF;
};

 TComputeColorStack lComputeColorStack[12];
vec4 ComputeColor(const in TRay aRay, const in vec2 aUV)
{
#   define MAX_BOUNCES 12
    THitRecord lRecord;
    lRecord.mUV = aUV;
    vec4 lColor = vec4(1.0);
    vec4 lEmissive = vec4(0.0);
    TRay lScatteredRay = aRay;
    
    float lScatteringPDF = 1.0f;

   
    uint lMaxStackFrames = 0;

    TOrthoNormalBases lCosinePDF;

    for(uint b = 0; b <= MAX_BOUNCES; b++)
    {
        if( b == MAX_BOUNCES)
        {
            //lScatteredRay.mDirection = vec3( -1.1,    1.0,  -2.0);
            //lScatteredRay.mDirection = vec3(0.f, 3.07f, -10.5f);
        }
        //lComputeColorStack[b].mColor = vec4(0);
        //lComputeColorStack[b].mEmissive = vec4(0);
        //lMaxStackFrames++;

        if(HitWorld(lScatteredRay, 0.001, FLT_MAX, lRecord))
        {
            vec4 lBounceAttenuation;
            TRay lBounceScatteredRay;
            float lPDF = FLT_MAX;
            if(BRDF(lScatteredRay, lRecord, lBounceAttenuation, lBounceScatteredRay, lPDF))
            {


                //if(b > 0)
   
                /*{
                    ONB_MakeFromNormal(lCosinePDF, lRecord.mNormal);
                    lBounceScatteredRay = TRay(lRecord.mPoint, CosinePDFGenerate(lCosinePDF, lRecord.mUV));
                    lPDF = CosinePDFValue(lCosinePDF, lBounceScatteredRay.mDirection);
                }*/

                //scattering pdf breaks refraction
                //lScatteringPDF *= ScatteringPDF(lScatteredRay, lRecord, lBounceScatteredRay) / lPDF;                
                lColor *= (lBounceAttenuation * lBounceAttenuation.a) + lBounceAttenuation;// * ScatteringPDF(lScatteredRay, lRecord, lBounceScatteredRay) / lPDF;
                //lComputeColorStack[b].mColor = lBounceAttenuation * ScatteringPDF(lScatteredRay, lRecord, lBounceScatteredRay) / lPDF;
                //lComputeColorStack[b].mEmissive.xyz = lBounceAttenuation.xyz * lBounceAttenuation.a;
                
      
                lScatteredRay = lBounceScatteredRay;


                
                //743
                if(lBounceAttenuation.a > 0.)
                {
                    //lEmissive.xyz += lBounceAttenuation.xyz * lBounceAttenuation.a;

                    break;
                }
            }
            else
            {
                //lEmissive.xyz += lBounceAttenuation.xyz * lBounceAttenuation.a;
                lColor *= (lBounceAttenuation * lBounceAttenuation.a);
                break;
            }
        }
        else
        {
            /*
            const vec3 lUnitDirection = normalize(lScatteredRay.mDirection);
            float lDistance = 0.5 * (lUnitDirection.y + 1.0);
            lEmissive += 0.0 * mix(vec4(1., 1., 1., 1.), vec4(0.5, 0.7, 1.0, 1.), lDistance);*/
            //lColor *= 0.f;
            lColor *= 0.f;
            break;
        }
    }
    
    //vec4 lOutputColor = vec4(1.);

    //for(uint b = 0; b <= MAX_BOUNCES; b++)
    /*for(uint b = lMaxStackFrames; b > 0; b--)
    {
        if(b > lMaxStackFrames)
        {
            //break;
        }

        lOutputColor *= lComputeColorStack[b].mEmissive + lComputeColorStack[b].mColor;
    }*/

    //return lEmissive * lColor;
    return lColor;

}

TRay GetCameraRay(const in vec2 aUV)
{
    vec3 lRandDisk = uFrameData.mCamera.mLensRadius * RandomPointInUnitDisk(aUV);
    vec3 lOffset = uFrameData.mCamera.mVecU.xyz * lRandDisk.x + uFrameData.mCamera.mVecV.xyz * lRandDisk.y;
    return TRay(uFrameData.mCamera.mOrigin.xyz + lOffset, 
                normalize(uFrameData.mCamera.mLowerLeftCorner.xyz + 
                aUV.x * uFrameData.mCamera.mHorizontal.xyz + 
                aUV.y * uFrameData.mCamera.mVertical.xyz - uFrameData.mCamera.mOrigin.xyz - lOffset)); 
                
}

void main(void)
{
    ivec2 lBufferCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 lColor = vec4(0);

    //for(int i = 0; i < 2; ++i)
    {
        vec2 lBaseUV = (vec2(lBufferCoord.xy) / vec2(gl_NumWorkGroups.xy)) + uFrameData.mTime;
        vec2 lUV = (vec2(lBufferCoord.xy) + vec2(NRand(lBaseUV), NRand(lBaseUV))) / vec2(gl_NumWorkGroups.xy);
    
        TRay lRay = GetCameraRay(lUV);
        vec4 lTempColor = ComputeColor(lRay, lUV);
        lColor += lTempColor;
    }
    //lColor /= 2.0;

    if(uFrameData.mSamplesCount > 0)
    {
        vec4 lInputColor = imageLoad(uRaytracerInputBuffer, lBufferCoord);
        lColor += lInputColor;
    }

    imageStore (uRaytracerOutputBuffer, lBufferCoord,  vec4(lColor.xyz, 1.0));
    //imageStore (uRaytracerOutputBuffer, lBufferCoord,  vec4(lUV.x, lUV.y, 1. - lUV.x  * lUV.y, 1.));
}
