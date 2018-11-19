#version 430
precision highp float;

// ---------------------
// INPUT LAYOUT
// ---------------------

layout(location = 0)  uniform ivec3     uResolution;           // viewport resolution (in pixels)


layout(binding = 0, rgba32f) uniform image2D uRaytracerOutputBuffer;
layout(binding = 1, rgba32f) uniform image2D uRaytracerInputBuffer;


layout(binding = 2) uniform TFrameData
{
    float mTime;
    uint  mSamplesCount;


    //Camera
    struct
    {
        vec4 mOrigin;
        vec4 mLowerLeftCorner;
        vec4 mHorizontal;
        vec4 mVertical;
        vec4 mVecU, mVecV, mVecW;
        float mLensRadius;
    } mCamera;
    
} uFrameData;

// ---------------------
// DEFINES
// ---------------------

#define FLT_MAX 99999999.99999
#define M_PI 3.141592654


// ---------------------
// RANDOMS
// ---------------------
float gSeed = 0;
float NRand(const in vec2 n) 
{
    float lValue = fract(sin(dot(n.xy + gSeed, vec2(12.9898, 78.233)))* 43758.5453);
    gSeed += lValue * 561.32898;
    return lValue;
}
/*
vec3 RandomPointInSphere(const in vec2 n)
{
   return 2.0 * vec3(NRand(n.xy + 68.36), NRand(n.yx - 32.46), NRand(n.yx * n.xy)) - vec3(1.0);
}
*/

vec3 RandomPointInSphere(const in vec2 n)
{
    vec3 p;
    for(int i = 0; i < 5; ++i)
    {
        p = 2.0 * vec3(NRand(n.xy + 68.36), NRand(n.yx - 32.46), NRand(n.yx * n.xy)) - vec3(1.0);
        if(dot(p,p) < 1.0)
            break;
    }

   return (p);
}

vec3 HemisphereSample_cos(const in vec2 aUV, out float aOutPhi) 
{
    const float kDitherLambda  = -.25 + .5 *  fract(52.9829189 * fract(dot(aUV, vec2(.06711056, .00583715))));
    aOutPhi         = NRand(aUV) * 2 * M_PI + kDitherLambda;
    const float cosTheta = sqrt(1. - NRand(aUV));
    const float sinTheta = sqrt(1. - cosTheta * cosTheta);

    return vec3(cos(aOutPhi) * sinTheta, sin(aOutPhi) * sinTheta, cosTheta);
}

vec3 RandomCosineDirection(const in vec2 aUV)
{
    const float r1 = NRand(aUV);
    const float r2 = NRand(aUV);
    const float z = sqrt(1. - r2);
    const float phi = 2. * M_PI * r1;
    const float x = cos(phi) * 2. * sqrt(r2);
    const float y = sin(phi) * 2. * sqrt(r2);
    return vec3(x, y, z);
}

vec3 RandomPointInUnitDisk(const in vec2 n)
{
    return 2.0 * vec3(NRand(n.xy + 68.36), NRand(n.yx - 32.46), 0.0) - vec3(1.0, 1.0, 0.0);
}


// ---------------------
// STRUCTS
// ---------------------

struct TRay
{
    vec3 mOrigin;
    vec3 mDirection;

#   define RayPointAt(_ray, _distance) (_ray.mOrigin + (_distance * _ray.mDirection))
};

struct THitRecord
{
    vec2  mUV;
    float mDistance;
    vec3 mPoint;
    vec3 mNormal;
    uint mMaterialId;
};

struct THitable
{
#   define HITABLE_SPHERE  0
#   define HITABLE_PLANE   1
#   define HITABLE_RM_MAP  2
    uint mType;
    uint mMaterialId;

    //sphere
    vec4 mParametersA;
    vec4 mParametersB;

    //plane
    //....

    //common
    //vec3 mRotation;
    

#   define MakeSphere(aMaterialId, aCenter, aRadius) THitable(HITABLE_SPHERE, aMaterialId, vec4(aCenter, aRadius), vec4(0.))
#   define MakePlane(aMaterialId, aComponents, aK, aType, aFlipNormal) THitable(HITABLE_PLANE, aMaterialId, aComponents, vec4(aK, aType, 0., aFlipNormal))
#   define MakeRaymarching() THitable(HITABLE_RM_MAP, 0, vec4(0.), vec4(0.))
};

struct TMaterial
{
    vec4 mAlbedo;
    float mRoughness;
    float mMetalness;
    float mRefraction;


    //int mColorRoughTex;

#   define MakeSolidMat(aAlbedo, aRoughness, aMetalness)                TMaterial(aAlbedo, aRoughness, aMetalness, 0.0f)
#   define MakeGlassMat(aAlbedo, aRoughness, aRefraction)               TMaterial(aAlbedo, aRoughness, 0.0f, aRefraction)
//#   define MakeSolidTexdMat(mColorRoughTex, aMetalness)                TMaterial(vec4(1., 1., 1., 0.), 1., aMetalness, 0., aColorTex, aRMRTexture)
//#   define MakeGlassTexdMat(mColorRoughTex, aRefraction)               TMaterial(vec4(1., 1., 1., 0.), 1., 0., aRefraction, aColorTex, aRMRTexture)

};

struct TOrthoNormalBases
{
    vec3 mAxisU;
    vec3 mAxisV;
    vec3 mAxisW;

//#   define ONB_Local(aONB, aPoint) ( aONB.mAxisU * aPoint.x + aONB.mAxisV * aPoint.y + aONB.mAxisW * aPoint.z )
};

vec3 ONB_Local(const in TOrthoNormalBases aONB, const in vec3 aPoint)
{
    return ( aONB.mAxisU * aPoint.x + aONB.mAxisV * aPoint.y + aONB.mAxisW * aPoint.z );
}

void ONB_MakeFromNormal(inout TOrthoNormalBases aOutONB, const in vec3 aNormal)
{
    aOutONB.mAxisW = normalize(aNormal);
    vec3 a = (abs(aOutONB.mAxisW.x) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    aOutONB.mAxisV = normalize(cross(aOutONB.mAxisW, a));
    aOutONB.mAxisU = cross(aOutONB.mAxisW, aOutONB.mAxisV);
}


float CosinePDFValue(const in TOrthoNormalBases aONB, const in vec3 aDirection)
{
    float lCosine = dot(normalize(aDirection), aONB.mAxisW);
    if(lCosine > 0.)
    {
        return lCosine / M_PI;
    }
    else
    {
        return 0.;
    }
}

vec3 CosinePDFGenerate(const in TOrthoNormalBases aONB, const in vec2 aUV)
{
    return ONB_Local(aONB, RandomCosineDirection(aUV));
}



// ---------------------
// GLOBALS
// ---------------------

#define MAX_HITABLES 1
const THitable gHitablesList[MAX_HITABLES] = THitable[]
(
    /*MakeSphere(1, vec3( 0.0,    0.0, -1.0),   0.5),
    MakeSphere(6, vec3( -1.1,    1.0,  -2.0),   0.3),
    MakeSphere(4, vec3(-1.1,    0.0, -1.0),   0.5),
    MakeSphere(4, vec3(-1.1,    0.0, -1.0),  -0.49),
    MakeSphere(2, vec3( 1.1,    0.0, -1.0),   0.5),
    MakeSphere(0, vec3( 0.0, -100.5, -1.0), 100.0),
    */

//    MakePlane(6, vec4( -2., 2., 1., 3.), -6.7, 0., -1.)
    MakeRaymarching() 
);


#define MAX_MATERIALS 8
const TMaterial gMaterialsList[MAX_MATERIALS] = TMaterial[]
(
    MakeSolidMat(vec4(1.0, 1.0, 1.0, 0.0), 1.0, 0.0),       //White Lmabert
    MakeSolidMat(vec4(0.2, 0.2, 0.8, 0.0), 1.0, 0.0),       //Blue Lambert
    MakeSolidMat(vec4(0.8f, 0.6f, 0.1f, 0.0), 0.1, 1.0),    //Gold
    MakeSolidMat(vec4(0.8f, 0.8f, 0.8f, 0.0), 0.0035, 1.0),   //Iron
    MakeGlassMat(vec4(1.0f, 1.0f, 1.0f, 0.0) * 1.0, 0.0, 1.7),    //White Glass
    MakeSolidMat(vec4(0.8, 0.2, 0.2, 0.), 1.0, 0.0),       //Red Lambert
    MakeSolidMat(vec4(1.0, 1.0, 1.0, 5.), 1.0, 0.0),       //White Light
    MakeGlassMat(vec4(1.0f, 0.0f, 0.0f, 0.0) * 0.95, 0.0, 1.7)    //Red Glass
);
//Other Materials
#define MAT_CORNELL_BOX 1000