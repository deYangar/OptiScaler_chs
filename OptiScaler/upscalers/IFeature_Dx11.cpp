#include <pch.h>
#include "IFeature_Dx11.h"
#include <State.h>

IFeature_Dx11::~IFeature_Dx11()
{
    if (State::Instance().isShuttingDown)
        return;

    Imgui.reset();
    OutputScaler.reset();
    RCAS.reset();
    Bias.reset();
}
