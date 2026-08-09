#include "pch.h"
#include "UpscalerTime_Dx11.h"

#include <State.h>

void UpscalerTimeDx11::Init(ID3D11Device* device)
{
    // Create Disjoint Query
    D3D11_QUERY_DESC disjointQueryDesc = {};
    disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

    // Create Timestamp Queries
    D3D11_QUERY_DESC timestampQueryDesc = {};
    timestampQueryDesc.Query = D3D11_QUERY_TIMESTAMP;

    for (int i = 0; i < QUERY_BUFFER_COUNT; i++)
    {
        device->CreateQuery(&disjointQueryDesc, &_disjointQueries[i]);
        device->CreateQuery(&timestampQueryDesc, &_startQueries[i]);
        device->CreateQuery(&timestampQueryDesc, &_endQueries[i]);
    }
}

void UpscalerTimeDx11::UpscaleStart(ID3D11DeviceContext* devieContext)
{
    _previousFrameIndex = (_currentFrameIndex + QUERY_BUFFER_COUNT - 2) % QUERY_BUFFER_COUNT;

    // Record the queries in the current frame
    devieContext->Begin(_disjointQueries[_currentFrameIndex]);
    devieContext->End(_startQueries[_currentFrameIndex]);
}

void UpscalerTimeDx11::UpscaleEnd(ID3D11DeviceContext* devieContext)
{
    devieContext->End(_endQueries[_currentFrameIndex]);
    devieContext->End(_disjointQueries[_currentFrameIndex]);

    _dx11UpscaleTrig[_currentFrameIndex] = true;
}

void UpscalerTimeDx11::ReadUpscalingTime(ID3D11DeviceContext* devieContext)
{
    // Retrieve the results from the previous frame
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
    if (devieContext->GetData(_disjointQueries[_previousFrameIndex], &disjointData, sizeof(disjointData), 0) == S_OK)
    {
        if (!disjointData.Disjoint && disjointData.Frequency > 0)
        {
            UINT64 startTime = 0, endTime = 0;
            if (devieContext->GetData(_startQueries[_previousFrameIndex], &startTime, sizeof(UINT64), 0) == S_OK &&
                devieContext->GetData(_endQueries[_previousFrameIndex], &endTime, sizeof(UINT64), 0) == S_OK)
            {
                double elapsedTimeMs = (endTime - startTime) / static_cast<double>(disjointData.Frequency) * 1000.0;

                // filter out posibly wrong measured high values
                if (elapsedTimeMs < 100.0)
                {
                    State::Instance().frameTimeMutex.lock();
                    State::Instance().upscaleTimes.push_back(elapsedTimeMs);
                    State::Instance().upscaleTimes.pop_front();
                    State::Instance().frameTimeMutex.unlock();
                }
            }
        }
    }

    _dx11UpscaleTrig[_currentFrameIndex] = false;
    _currentFrameIndex = (_currentFrameIndex + 1) % QUERY_BUFFER_COUNT;
}
