
#ifndef RT_CCAMERA_H
#define RT_CCAMERA_H

class CCamera
{
public:
    CCamera();

    CCamera(const TVec3& aOrigin,
            const TVec3& aLookAt,
            const TVec3& aViewUp,
            float aFOV,
            float aAspect,
            float aAperture,
            float aFocusDist);

    void SetFrame(const TVec3& aOrigin,
                  const TVec3& aLookAt,
                  const TVec3& aViewUp,
                  float aFOV,
                  float aAspect,
                  float aAperture,
                  float aFocusDist);

    
    const TVec3& GetOrigin() const { return mOrigin; }
    const TVec3& GetLookAt() const { return mLookAt; }
    const TVec3& GetViewUp() const { return mViewUp; }
    float GetFOV() const { return mFOV; }
    float GetAspect() const { return mAspect; }
    float GetAperture() const { return mAperture; }
    float GetFocusDist() const { return mFocusDist; }
    bool GetUpdated() const { return mUpdated; }

    void SetOrigin(const TVec3& aOrigin) { mOrigin = aOrigin; Update(); }
    void SetLookAt(const TVec3& aLookAt) { mLookAt = aLookAt; Update(); }
    void SetViewUp(const TVec3& aViewUp) { mViewUp = aViewUp; Update(); }
    void SetFOV(float aFOV) { mFOV = aFOV; Update(); }
    void SetAspect(float aAspect) { mAspect = aAspect; Update(); }
    void SetAperture(float aAperture) { mAperture = aAperture; Update(); }
    void SetFocusDist(float aFocusDist) { mFocusDist = aFocusDist; Update(); }
    void GetUpdated(bool aUpdated) { mUpdated = aUpdated; }

    void Update();
    

    bool PurgueShaderValues(float* aValues);

private:
    //store only
    TVec3 mLookAt;
    TVec3 mViewUp;
    float mFOV;
    float mAspect;
    float mAperture;
    float mFocusDist;
    bool mUpdated;

    //pass to shader
    TVec3 mOrigin;
    TVec3 mLowerLeftCorner;
    TVec3 mHorizontal;
    TVec3 mVertical;
    TVec3 mVecU, mVecV, mVecW;
    float mLensRadius;
    
};



#endif //RT_CCAMERA_H