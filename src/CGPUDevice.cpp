
#include "Common.h"
#include <Windows.h>
#include "CGPUDevice.h"
#include <stdio.h>

#include <glad/glad.h>
#include <glad/glad_wgl.h>

#include "TGPUStructs.h"
#include "CCamera.h"

CGPUDevice* CGPUDevice::GetInstance()
{
    static CGPUDevice sDevice;
    return &sDevice;
}


struct CGPUDevice::TData
{
    //programs
    TGPUHandle mRaytracerProgram;
    TGPUHandle mOutputProgram;

    //buffers
    TGPUHandle mRaytracerOutputBufferPair[2];
    
    //runtime
    uint32_t mCurrentRTBuffer;
    uint32_t mViewportWidth;
    uint32_t mViewportHeight;
       
    TFrameData mFrameData;
    TGPUHandle mFrameDataBuffer;

    CCamera mCamera;

    TData()
        : mRaytracerProgram(0)
        , mOutputProgram(0)
        , mCurrentRTBuffer(0)
        , mViewportWidth(0)
        , mViewportHeight(0)

    {
        mRaytracerOutputBufferPair[0] = 0;
        mRaytracerOutputBufferPair[1] = 0;
    }

    ~TData()
    {
        CleanUp();
    }

    void InitFrameData()
    {
        mFrameData.mTime = 0.f;
        mFrameData.mSamplesCount = 0;

        /*mFrameData.mCamera = 
        {
            {0, 0.5, 2},      //vec3 mOrigin;
            {-2, -1, -1},   //vec3 mLowerLerftCorner;
            {4, 0, 0},      //vec3 mHorizontal;
            {0, 2, 0},      //vec3 mVertical;
        };*/

        //const TVec3 lOrigin(-3., 0.5f, 2);
        const TVec3 lOrigin(0, 0.0f, 1);
        const TVec3 lLookAt(0, 0.0, -1);
        const TVec3 lViewUp(0, 1, 0);

        const float lAperture = 0.0f;
        const float lDistToFocus =  3.5;
        const float lFov = 40.0f;

        /*
         const TVec3 lOrigin(278, 278, -800);
        const TVec3 lLookAt(278, 278, 0);
        const TVec3 lViewUp(0, 1, 0);

        const float lAperture = 0.0f;
        const float lDistToFocus =  10;
        const float lFov = 40.0f;
        
        */

        mCamera.SetFrame
        (
            lOrigin,
            lLookAt,
            lViewUp,
            lFov,
            float(mViewportWidth) / float(mViewportHeight),
            lAperture,
            lDistToFocus//length(lOrigin - lLookAt) * 
        );
    }

    void CleanUp()
    {
        glDeleteProgram(mRaytracerProgram);
        glDeleteProgram(mOutputProgram);
        glDeleteBuffers(2, mRaytracerOutputBufferPair);

        mRaytracerProgram = 0;
        mOutputProgram = 0;
        mRaytracerOutputBufferPair[0] = 0;
        mRaytracerOutputBufferPair[1] = 0;
    }

    TGPUHandle LoadShader(const char** aPaths, uint32_t aNumFiles, GLenum aType)
    {
        WriteLog("Loading shader %s...", aPaths[aNumFiles - 1]);

        TGPUHandle lHandle = (TGPUHandle)-1;
        char** lBuffers = new char*[aNumFiles];

        for(uint32_t i = 0; i < aNumFiles; ++i)
        {
            size_t lLength = 0;
            FILE * lFile = fopen(aPaths[i], "rb");
        

            if (lFile)
            {
                fseek(lFile, 0, SEEK_END);
                lLength = ftell(lFile);
                fseek(lFile, 0, SEEK_SET);
                lBuffers[i] = new char[lLength];
                if (lBuffers[i])
                {
                    fread(lBuffers[i], 1, lLength, lFile);
                }
                lBuffers[i][lLength - 1] = 0;
                fclose(lFile);
            }
        }

        //if (lBuffers)
        {
            lHandle = glCreateShaderProgramv( aType, aNumFiles, lBuffers );
            
            for(uint32_t i = 0; i < aNumFiles; ++i)
            {
                delete [] lBuffers[i];
            }
            delete [] lBuffers;

            char lLog  [65536];
            GLsizei lLogLength = 0;
            glGetProgramInfoLog(lHandle, sizeof(lLog), &lLogLength, lLog);
            if (0 != lLogLength)
            {
                WriteLog("Error linking program!. Compiler said:\n%s[EndOfErrorMessage]", lLog);
            }
        }


        return lHandle;
    }

    TGPUHandle CreateTexture(uint32_t aWidth, uint32_t aHeight)
    {
        TGPUHandle lHandle;

        glGenTextures(1, &lHandle);
        glBindTexture(GL_TEXTURE_2D, lHandle);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, aWidth, aHeight, 0, GL_RGBA, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        return lHandle;
    }

    void LoadShaders()
    {
        const char* lOutputShaderFiles[] =
        {
            "..\\src\\shaders\\common.h.glsl",
            "..\\src\\shaders\\output.frag.glsl"
        };
        mOutputProgram = LoadShader(lOutputShaderFiles, 2, GL_FRAGMENT_SHADER);

        const char* lRaytracerShaderFiles[] =
        {
            "..\\src\\shaders\\common.h.glsl",
            "..\\src\\shaders\\raymarching.h.glsl",
            "..\\src\\shaders\\raytracer.comp.glsl"
        };
        mRaytracerProgram = LoadShader(lRaytracerShaderFiles, 3, GL_COMPUTE_SHADER);
    }

    void InitBuffers()
    {
        mRaytracerOutputBufferPair[0] = CreateTexture(mViewportWidth, mViewportHeight);
        mRaytracerOutputBufferPair[1] = CreateTexture(mViewportWidth, mViewportHeight);

        glGenBuffers        (1, &mFrameDataBuffer);
        glBindBuffer        (GL_UNIFORM_BUFFER, mFrameDataBuffer);
        glBindBufferBase    (GL_UNIFORM_BUFFER, eub_uFrameData, mFrameDataBuffer);
        glBufferData        (GL_UNIFORM_BUFFER, sizeof(TFrameData), &mFrameData, GL_STREAM_DRAW);
        glBindBuffer        (GL_UNIFORM_BUFFER, 0);
    }
};

CGPUDevice::CGPUDevice()
    : mData(0)
{
    mData = new CGPUDevice::TData();
}

CGPUDevice::~CGPUDevice()
{
    if(0 != mData)
    {
        delete mData;
        mData = 0;
    }
}

bool CGPUDevice::Init(uint32_t aViewportWidth, uint32_t aViewportHeight, HDC hDC)
{
    mData->mViewportWidth = aViewportWidth;
    mData->mViewportHeight = aViewportHeight;

    mData->InitFrameData();

    if(!gladLoadGL()) 
    {
        WriteLog("Something went wrong in gladLoadGL()!");
        return false;
    }

    if(!gladLoadWGL(hDC)) 
    {
        WriteLog("Something went wrong in gladLoadWGL()!");
        return false;
    }

    wglSwapIntervalEXT(0);

    {
        WriteLog("Driver information:");
        WriteLog("    GL_VERSION.................: %s.", reinterpret_cast< const char* >(glGetString(GL_VERSION)));
        WriteLog("    GL_VENDOR..................: %s.", reinterpret_cast< const char* >(glGetString(GL_VENDOR)));
        WriteLog("    GL_RENDERER................: %s.", reinterpret_cast< const char* >(glGetString(GL_RENDERER)));
        WriteLog("    GL_SHADING_LANGUAGE_VERSION: %s.", reinterpret_cast< const char* >(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    }

    //init all shaders and buffers
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 0.0f, 0.5f, 1.0f);

    mData->LoadShaders(); 
    mData->InitBuffers();


    return true;
}

void CGPUDevice::End()
{

}

void CGPUDevice::ResetFrame()
{
    mData->mFrameData.mSamplesCount = 0;
}

void CGPUDevice::AddCameraXYZ(float x, float y, float z)
{
    TVec3 lVar = mData->mCamera.GetLookAt();
    lVar.x += x*0.01f;
    lVar.y += -y*0.01f;
    lVar.z += z*0.01f;
    mData->mCamera.SetLookAt(lVar);

    //mData->mCamera.SetAperture( mData->mCamera.GetAperture() + y * 0.01);
    //mData->mCamera.SetFocusDist( mData->mCamera.GetFocusDist() + y * 0.01);
}

uint32_t CGPUDevice::GetSamplesCount() const
{
    return mData->mFrameData.mSamplesCount;
}

void CGPUDevice::ExecuteComputeStep(float aTotalTime)
{
    mData->mFrameData.mTime = aTotalTime;
    //update camera
    if(mData->mCamera.PurgueShaderValues((float*)&mData->mFrameData.mCamera))
    {
        ResetFrame();
    }

    //copy frame data buffer
    glBindBuffer    (GL_UNIFORM_BUFFER, mData->mFrameDataBuffer);
    glBufferSubData (GL_UNIFORM_BUFFER, 0, sizeof(TFrameData), &mData->mFrameData);
    glBindBuffer    (GL_UNIFORM_BUFFER, 0);

    //execute compute shader step
    glUseProgram(mData->mRaytracerProgram);
    glBindImageTexture(0, mData->mRaytracerOutputBufferPair[mData->mCurrentRTBuffer], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, mData->mRaytracerOutputBufferPair[1 - mData->mCurrentRTBuffer], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glDispatchCompute(mData->mViewportWidth, mData->mViewportHeight, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUseProgram(0);

    mData->mFrameData.mSamplesCount++;
    mData->mCurrentRTBuffer = 1 - mData->mCurrentRTBuffer;
}
    
void CGPUDevice::Render(float aTotalTime)
{
    mData->mCurrentRTBuffer = 1 - mData->mCurrentRTBuffer;

    //display the image generated by compute and apply post process
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer    (GL_UNIFORM_BUFFER, mData->mFrameDataBuffer);
    glBufferSubData (GL_UNIFORM_BUFFER, 0, sizeof(TFrameData), &mData->mFrameData);
    glBindBuffer    (GL_UNIFORM_BUFFER, 0);

    glUseProgram(mData->mOutputProgram);
    glUniform3i(eul_uResolution, mData->mViewportWidth,  mData->mViewportHeight, 0);
    glBindImageTexture(0, mData->mRaytracerOutputBufferPair[mData->mCurrentRTBuffer], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
    glRects(-1, -1, 1, 1);

    
    mData->mCurrentRTBuffer = 1 - mData->mCurrentRTBuffer;

}