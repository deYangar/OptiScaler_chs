#pragma once

#include "SysUtils.h"

#include <d3d11.h>

class UpscalerTimeDx11
{
  public:
    static void Init(ID3D11Device* device);
    static void UpscaleStart(ID3D11DeviceContext* devieContext);
    static void UpscaleEnd(ID3D11DeviceContext* devieContext);
    static void ReadUpscalingTime(ID3D11DeviceContext* devieContext);

  private:
    inline static const int QUERY_BUFFER_COUNT = 3;
    inline static ID3D11Query* _disjointQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
    inline static ID3D11Query* _startQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
    inline static ID3D11Query* _endQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };

    inline static bool _dx11UpscaleTrig[QUERY_BUFFER_COUNT] = { false, false, false };
    inline static int _currentFrameIndex = 0;
    inline static int _previousFrameIndex = 0;
};
