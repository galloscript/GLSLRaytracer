

#include "Common.h"
#include "CCamera.h"

#include <string.h>

CCamera::CCamera()
    : mUpdated(false)
{;}

CCamera::CCamera(   const TVec3& aOrigin,
                    const TVec3& aLookAt,
                    const TVec3& aViewUp,
                    float aFOV,
                    float aAspect,
                    float aAperture,
                    float aFocusDist)
    : mOrigin(aOrigin)
    , mUpdated(false)
{
    SetFrame(aOrigin, aLookAt, aViewUp, aFOV, aAspect, aAperture, aFocusDist);
}

void CCamera::SetFrame( const TVec3& aOrigin,
                        const TVec3& aLookAt,
                        const TVec3& aViewUp,
                        float aFOV,
                        float aAspect,
                        float aAperture,
                        float aFocusDist)
{
    mOrigin = aOrigin;
    mLookAt = aLookAt;
    mViewUp = aViewUp;
    mFOV = aFOV;
    mAspect = aAspect;
    mAperture = aAperture;
    mFocusDist = aFocusDist;

    Update();
}

void CCamera::Update()
{
    float lTheta = mFOV * float(M_PI) / 180.f;
    float lHalfHeight = tanf(lTheta * 0.5f);
    float lHalfWidth = mAspect * lHalfHeight;
    mLensRadius = mAperture * 0.5f;
    mVecW = normalize(mOrigin - mLookAt);
    mVecU = normalize(cross(mViewUp, mVecW));
    mVecV = cross(mVecW, mVecU);
    mLowerLeftCorner = mOrigin - (lHalfWidth * mFocusDist * mVecU) - (lHalfHeight * mFocusDist * mVecV) - (mFocusDist * mVecW);
    mHorizontal = 2.0f * lHalfWidth * mFocusDist * mVecU;
    mVertical = 2.0f * lHalfHeight * mFocusDist * mVecV;

    mUpdated = true;
}

/*
const TRay CCamera::GetRay(vec2 aUV) const
{
    TVec3 lRandDisk = mLensRadius * RandomPointInUnitDisk(aUV);
    TVec3 lOffset = mVecU * lRandDisk.x + mVecV * lRandDisk.y;
    //float lTime = mTimeA * aTimeDelta * (mTimeB - mTimeA);
    return TRay(mOrigin + lOffset, 
                normalize(mLowerLeftCorner + 
                aUCoord * mHorizontal + 
                aVCoord * mVertical - mOrigin - lOffset)); 
}
*/

bool CCamera::PurgueShaderValues(float* aValues)
{
    if(mUpdated)
    {
        float lOutput[] =
        {
            mOrigin.x, mOrigin.y, mOrigin.z, 0.0f,
            mLowerLeftCorner.x, mLowerLeftCorner.y, mLowerLeftCorner.z, 0.0f,
            mHorizontal.x, mHorizontal.y, mHorizontal.z, 0.0f,
            mVertical.x, mVertical.y, mVertical.z, 0.0f,
            mVecU.x, mVecU.y, mVecU.z, 0.0f,
            mVecV.x, mVecV.y, mVecV.z, 0.0f,
            mVecW.x, mVecW.y, mVecW.z, 0.0f,
            mLensRadius
        };

        memcpy(aValues, lOutput, sizeof(lOutput));
        mUpdated = false;
        return true;
    }

    return false;
}