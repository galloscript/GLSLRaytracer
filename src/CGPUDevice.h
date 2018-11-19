    
#pragma once

class CGPUDevice
{
public:
    static CGPUDevice* GetInstance();

    CGPUDevice();
    ~CGPUDevice();

    bool Init(uint32_t aViewportWidth, uint32_t aViewportHeight, HDC hDC);
    void End();

    void ResetFrame();
    void AddCameraXYZ(float x, float y, float z);

    uint32_t GetSamplesCount() const;

    void ExecuteComputeStep(float aTotalTime);
    
    void Render(float aTotalTime);
private:
    struct TData;
    TData* mData;
};

#define RT_GPU() CGPUDevice::GetInstance()