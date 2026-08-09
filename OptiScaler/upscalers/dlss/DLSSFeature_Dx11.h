#pragma once
#include <upscalers/IFeature_Dx11.h>
#include "DLSSFeature.h"
#include <string>

class DLSSFeatureDx11 : public DLSSFeature, public IFeature_Dx11
{
  private:
  protected:
  public:
    bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) override;
    bool Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters) override;

    feature_version Version() override { return DLSSFeature::Version(); }
    Upscaler GetUpscalerType() const final { return DLSSFeature::GetUpscalerType(); }

    bool IsWithDx12() override { return false; }

    DLSSFeatureDx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
    ~DLSSFeatureDx11();
};
