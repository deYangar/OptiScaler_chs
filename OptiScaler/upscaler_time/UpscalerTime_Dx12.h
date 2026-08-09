#pragma once

#include "SysUtils.h"

#include <d3d12.h>

class UpscalerTimeDx12
{
  public:
    static void Init(ID3D12Device* device);
    static void UpscaleStart(ID3D12GraphicsCommandList* cmdList);
    static void UpscaleEnd(ID3D12GraphicsCommandList* cmdList);
    static void ReadUpscalingTime(ID3D12CommandQueue* commandQueue);

  private:
    static inline ID3D12QueryHeap* _queryHeap = nullptr;
    static inline ID3D12Resource* _readbackBuffer = nullptr;
    static inline bool _dx12UpscaleTrig = false;
};
