

typedef uint32_t TGPUHandle;

enum EUniformLocation
{
    eul_uResolution = 0,
};

enum EUniformBindings
{
    eub_uFrameData = 2,
};

struct TCamera
{
    TVec3 mOrigin;
    TVec3 mLowerLeftCorner;
    TVec3 mHorizontal;
    TVec3 mVertical;
    TVec3 mVecU, mVecV, mVecW;
    float mLensRadius;
    float mTimeA, mTimeB;
};

struct TComputeProgramData
{
    TGPUHandle mComputeProgram;
    TCamera mCamera;
};


struct vec4 { float x,y,z,w; };


struct TFrameData
{
    float    mTime;
    uint32_t mSamplesCount;
    float _padding[2];

    //Camera
    struct
    {
        vec4 mOrigin;
        vec4 mLowerLerftCorner;
        vec4 mHorizontal;
        vec4 mVertical;
        vec4 mVecU, mVecV, mVecW;
        float mLensRadius;
        float _padding[2];
    } mCamera;
};
