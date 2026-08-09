#include "pch.h"

#include "RCAS_Vk.h"

#include "precompile/RCAS_Shader_Vk.h"
#include "precompile/da_das_sharpen_Shader_Vk.h"
#include "precompile/da_rcas_sharpen_Shader_Vk.h"

#include <Config.h>

RCAS_Vk::RCAS_Vk(std::string InName, VkDevice InDevice, VkPhysicalDevice InPhysicalDevice)
    : Shader_Vk(InName, InDevice, InPhysicalDevice)
{
    if (InDevice == VK_NULL_HANDLE)
    {
        LOG_ERROR("InDevice is nullptr!");
        return;
    }

    LOG_FUNC();

    CreateDescriptorSetLayout();
    CreateDescriptorSetLayoutDA(_descriptorSetLayoutDA, _pipelineLayoutDA);
    CreateDescriptorSetLayoutDA(_descriptorSetLayoutDASDA, _pipelineLayoutDASDA);
    CreateConstantBuffer();
    CreateConstantBufferDA(_constantBufferDA, _constantBufferMemoryDA, _mappedConstantBufferDA);
    CreateConstantBufferDA(_constantBufferDASDA, _constantBufferMemoryDASDA, _mappedConstantBufferDASDA);
    CreateDescriptorPool();
    CreateDescriptorPoolDA(_descriptorPoolDA);
    CreateDescriptorPoolDA(_descriptorPoolDASDA);
    CreateDescriptorSets();
    CreateDescriptorSetsDA(_descriptorSetLayoutDA, _descriptorPoolDA, _descriptorSetsDA);
    CreateDescriptorSetsDA(_descriptorSetLayoutDASDA, _descriptorPoolDASDA, _descriptorSetsDASDA);

    // Create nearest sampler
    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(_device, &samplerInfo, nullptr, &_nearestSampler);

    std::vector<char> shaderCode(rcas_spv, rcas_spv + sizeof(rcas_spv));
    if (!CreateComputePipeline(_device, _pipelineLayout, &_pipeline, shaderCode))
    {
        LOG_ERROR("Failed to create pipeline for RCAS_Vk");
        _init = false;
        return;
    }

    std::vector<char> shaderCodeDA(da_rcas_sharpen_spv, da_rcas_sharpen_spv + sizeof(da_rcas_sharpen_spv));
    if (!CreateComputePipeline(_device, _pipelineLayoutDA, &_pipelineDA, shaderCodeDA))
    {
        LOG_ERROR("Failed to create pipeline for RCAS_Vk depth adaptive");
        _init = false;
        return;
    }

    std::vector<char> shaderCodeDASDA(da_das_sharpen_spv, da_das_sharpen_spv + sizeof(da_das_sharpen_spv));
    if (!CreateComputePipeline(_device, _pipelineLayoutDASDA, &_pipelineDASDA, shaderCodeDASDA))
    {
        LOG_ERROR("Failed to create pipeline for RCAS_Vk DAS depth aware");
        _init = false;
        return;
    }

    _init = true;
}

RCAS_Vk::~RCAS_Vk()
{
    if (_pipelineDA != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(_device, _pipelineDA, nullptr);
        _pipelineDA = VK_NULL_HANDLE;
    }

    if (_pipelineDASDA != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(_device, _pipelineDASDA, nullptr);
        _pipelineDASDA = VK_NULL_HANDLE;
    }

    if (_descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
        _descriptorPool = VK_NULL_HANDLE;
    }

    if (_descriptorPoolDA != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(_device, _descriptorPoolDA, nullptr);
        _descriptorPoolDA = VK_NULL_HANDLE;
    }

    if (_descriptorPoolDASDA != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(_device, _descriptorPoolDASDA, nullptr);
        _descriptorPoolDASDA = VK_NULL_HANDLE;
    }

    if (_descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
        _descriptorSetLayout = VK_NULL_HANDLE;
    }

    if (_descriptorSetLayoutDA != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(_device, _descriptorSetLayoutDA, nullptr);
        _descriptorSetLayoutDA = VK_NULL_HANDLE;
    }

    if (_descriptorSetLayoutDASDA != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(_device, _descriptorSetLayoutDASDA, nullptr);
        _descriptorSetLayoutDASDA = VK_NULL_HANDLE;
    }

    if (_pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
        _pipelineLayout = VK_NULL_HANDLE;
    }

    if (_pipelineLayoutDA != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(_device, _pipelineLayoutDA, nullptr);
        _pipelineLayoutDA = VK_NULL_HANDLE;
    }

    if (_pipelineLayoutDASDA != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(_device, _pipelineLayoutDASDA, nullptr);
        _pipelineLayoutDASDA = VK_NULL_HANDLE;
    }

    if (_constantBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(_device, _constantBuffer, nullptr);
        _constantBuffer = VK_NULL_HANDLE;
    }

    if (_constantBufferDA != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(_device, _constantBufferDA, nullptr);
        _constantBufferDA = VK_NULL_HANDLE;
    }

    if (_constantBufferDASDA != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(_device, _constantBufferDASDA, nullptr);
        _constantBufferDASDA = VK_NULL_HANDLE;
    }

    if (_constantBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(_device, _constantBufferMemory, nullptr);
        _constantBufferMemory = VK_NULL_HANDLE;
    }

    if (_constantBufferMemoryDA != VK_NULL_HANDLE)
    {
        vkFreeMemory(_device, _constantBufferMemoryDA, nullptr);
        _constantBufferMemoryDA = VK_NULL_HANDLE;
    }

    if (_constantBufferMemoryDASDA != VK_NULL_HANDLE)
    {
        vkFreeMemory(_device, _constantBufferMemoryDASDA, nullptr);
        _constantBufferMemoryDASDA = VK_NULL_HANDLE;
    }

    if (_nearestSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(_device, _nearestSampler, nullptr);
        _nearestSampler = VK_NULL_HANDLE;
    }

    ReleaseImageResource();
}

void RCAS_Vk::CreateDescriptorSetLayout()
{
    // Binding 0: ConstantBuffer
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 1: Source (Sampled Image)
    VkDescriptorSetLayoutBinding sourceLayoutBinding {};
    sourceLayoutBinding.binding = 1;
    sourceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sourceLayoutBinding.descriptorCount = 1;
    sourceLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 2: Motion (Sampled Image)
    VkDescriptorSetLayoutBinding motionLayoutBinding {};
    motionLayoutBinding.binding = 2;
    motionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    motionLayoutBinding.descriptorCount = 1;
    motionLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 3: Dest (Storage Image)
    VkDescriptorSetLayoutBinding destLayoutBinding {};
    destLayoutBinding.binding = 3;
    destLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    destLayoutBinding.descriptorCount = 1;
    destLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, sourceLayoutBinding, motionLayoutBinding,
                                                           destLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create descriptor set layout!");
        return;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create pipeline layout!");
    }
}

void RCAS_Vk::CreateDescriptorSetLayoutDA(VkDescriptorSetLayout& descriptorSetLayout, VkPipelineLayout& pipelineLayout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding sourceLayoutBinding {};
    sourceLayoutBinding.binding = 1;
    sourceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sourceLayoutBinding.descriptorCount = 1;
    sourceLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding motionLayoutBinding {};
    motionLayoutBinding.binding = 2;
    motionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    motionLayoutBinding.descriptorCount = 1;
    motionLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding depthLayoutBinding {};
    depthLayoutBinding.binding = 3;
    depthLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthLayoutBinding.descriptorCount = 1;
    depthLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding destLayoutBinding {};
    destLayoutBinding.binding = 4;
    destLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    destLayoutBinding.descriptorCount = 1;
    destLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, sourceLayoutBinding, motionLayoutBinding,
                                                           depthLayoutBinding, destLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create depth adaptive descriptor set layout!");
        return;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create depth adaptive pipeline layout!");
    }
}

void RCAS_Vk::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(2 * MAX_FRAMES_IN_FLIGHT) },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) }
    };

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create descriptor pool!");
    }
}

void RCAS_Vk::CreateDescriptorPoolDA(VkDescriptorPool& descriptorPool)
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(3 * MAX_FRAMES_IN_FLIGHT) },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) }
    };

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create depth adaptive descriptor pool!");
    }
}

void RCAS_Vk::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
    {
        LOG_ERROR("failed to allocate descriptor sets!");
    }
}

void RCAS_Vk::CreateDescriptorSetsDA(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool,
                                     std::vector<VkDescriptorSet>& descriptorSets)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        LOG_ERROR("failed to allocate depth adaptive descriptor sets!");
    }
}

void RCAS_Vk::UpdateDescriptorSet(VkCommandBuffer cmdList, int setIndex, VkImageView inputView, VkImageView motionView,
                                  VkImageView outputView)
{
    (void) cmdList;

    // Check if motion view invalid, if so use input view
    if (motionView == VK_NULL_HANDLE)
        motionView = inputView;

    VkDescriptorSet descriptorSet = _descriptorSets[setIndex];

    // 0: UBO
    VkDescriptorBufferInfo bufferInfo {};
    bufferInfo.buffer = _constantBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(InternalConstants);

    VkWriteDescriptorSet descriptorWriteUBO {};
    descriptorWriteUBO.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteUBO.dstSet = descriptorSet;
    descriptorWriteUBO.dstBinding = 0;
    descriptorWriteUBO.dstArrayElement = 0;
    descriptorWriteUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWriteUBO.descriptorCount = 1;
    descriptorWriteUBO.pBufferInfo = &bufferInfo;

    // 1: Source
    VkDescriptorImageInfo sourceInfo {};
    sourceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sourceInfo.imageView = inputView;
    sourceInfo.sampler = _nearestSampler; // Instead of VK_NULL_HANDLE

    VkWriteDescriptorSet descriptorWriteSource {};
    descriptorWriteSource.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteSource.dstSet = descriptorSet;
    descriptorWriteSource.dstBinding = 1;
    descriptorWriteSource.dstArrayElement = 0;
    descriptorWriteSource.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWriteSource.descriptorCount = 1;
    descriptorWriteSource.pImageInfo = &sourceInfo;

    // 2: Motion
    VkDescriptorImageInfo motionInfo {};
    motionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    motionInfo.imageView = motionView;
    motionInfo.sampler = _nearestSampler;

    VkWriteDescriptorSet descriptorWriteMotion {};
    descriptorWriteMotion.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteMotion.dstSet = descriptorSet;
    descriptorWriteMotion.dstBinding = 2;
    descriptorWriteMotion.dstArrayElement = 0;
    descriptorWriteMotion.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWriteMotion.descriptorCount = 1;
    descriptorWriteMotion.pImageInfo = &motionInfo;

    // 3: Dest
    VkDescriptorImageInfo destInfo {};
    destInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // RWTexture usually needs GENERAL
    destInfo.imageView = outputView;
    destInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet descriptorWriteDest {};
    descriptorWriteDest.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteDest.dstSet = descriptorSet;
    descriptorWriteDest.dstBinding = 3;
    descriptorWriteDest.dstArrayElement = 0;
    descriptorWriteDest.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWriteDest.descriptorCount = 1;
    descriptorWriteDest.pImageInfo = &destInfo;

    std::vector<VkWriteDescriptorSet> descriptorWritesBuffer = { descriptorWriteUBO, descriptorWriteSource,
                                                                 descriptorWriteMotion, descriptorWriteDest };

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWritesBuffer.size()), descriptorWritesBuffer.data(),
                           0, nullptr);
}

void RCAS_Vk::UpdateDescriptorSetDA(VkDescriptorSet descriptorSet, VkBuffer constantBuffer, VkImageView inputView,
                                    VkImageView motionView, VkImageView depthView, VkImageView outputView)
{
    if (motionView == VK_NULL_HANDLE)
        motionView = inputView;

    VkDescriptorBufferInfo bufferInfo {};
    bufferInfo.buffer = constantBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(InternalConstantsDA);

    VkWriteDescriptorSet descriptorWriteUBO {};
    descriptorWriteUBO.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteUBO.dstSet = descriptorSet;
    descriptorWriteUBO.dstBinding = 0;
    descriptorWriteUBO.dstArrayElement = 0;
    descriptorWriteUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWriteUBO.descriptorCount = 1;
    descriptorWriteUBO.pBufferInfo = &bufferInfo;

    VkDescriptorImageInfo sourceInfo {};
    sourceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sourceInfo.imageView = inputView;
    sourceInfo.sampler = _nearestSampler;

    VkWriteDescriptorSet descriptorWriteSource {};
    descriptorWriteSource.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteSource.dstSet = descriptorSet;
    descriptorWriteSource.dstBinding = 1;
    descriptorWriteSource.dstArrayElement = 0;
    descriptorWriteSource.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWriteSource.descriptorCount = 1;
    descriptorWriteSource.pImageInfo = &sourceInfo;

    VkDescriptorImageInfo motionInfo {};
    motionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    motionInfo.imageView = motionView;
    motionInfo.sampler = _nearestSampler;

    VkWriteDescriptorSet descriptorWriteMotion {};
    descriptorWriteMotion.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteMotion.dstSet = descriptorSet;
    descriptorWriteMotion.dstBinding = 2;
    descriptorWriteMotion.dstArrayElement = 0;
    descriptorWriteMotion.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWriteMotion.descriptorCount = 1;
    descriptorWriteMotion.pImageInfo = &motionInfo;

    VkDescriptorImageInfo depthInfo {};
    depthInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthInfo.imageView = depthView;
    depthInfo.sampler = _nearestSampler;

    VkWriteDescriptorSet descriptorWriteDepth {};
    descriptorWriteDepth.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteDepth.dstSet = descriptorSet;
    descriptorWriteDepth.dstBinding = 3;
    descriptorWriteDepth.dstArrayElement = 0;
    descriptorWriteDepth.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWriteDepth.descriptorCount = 1;
    descriptorWriteDepth.pImageInfo = &depthInfo;

    VkDescriptorImageInfo destInfo {};
    destInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    destInfo.imageView = outputView;
    destInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet descriptorWriteDest {};
    descriptorWriteDest.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteDest.dstSet = descriptorSet;
    descriptorWriteDest.dstBinding = 4;
    descriptorWriteDest.dstArrayElement = 0;
    descriptorWriteDest.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWriteDest.descriptorCount = 1;
    descriptorWriteDest.pImageInfo = &destInfo;

    std::vector<VkWriteDescriptorSet> descriptorWritesBuffer = { descriptorWriteUBO, descriptorWriteSource,
                                                                 descriptorWriteMotion, descriptorWriteDepth,
                                                                 descriptorWriteDest };

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWritesBuffer.size()), descriptorWritesBuffer.data(),
                           0, nullptr);
}

bool RCAS_Vk::DispatchRCAS(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                           VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo, VkImageInfo* OutResourceInfo)
{
    (void) InDevice;

    InternalConstants constants {};

    constants.OutputWidth = OutResourceInfo->Width;
    constants.OutputHeight = OutResourceInfo->Height;
    constants.MotionWidth = InMotionVectorsInfo->Width;
    constants.MotionHeight = InMotionVectorsInfo->Height;

    FillMotionConstants(constants, InConstants);

    if (_mappedConstantBuffer)
    {
        memcpy(_mappedConstantBuffer, &constants, sizeof(InternalConstants));
    }

    _currentSetIndex = (_currentSetIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    UpdateDescriptorSet(InCmdList, _currentSetIndex, InResourceInfo->ImageView, InMotionVectorsInfo->ImageView,
                        OutResourceInfo->ImageView);

    vkCmdBindPipeline(InCmdList, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);

    vkCmdBindDescriptorSets(InCmdList, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1,
                            &_descriptorSets[_currentSetIndex], 0, nullptr);

    uint32_t groupX = (constants.OutputWidth + 15) / 16;
    uint32_t groupY = (constants.OutputHeight + 15) / 16;
    vkCmdDispatch(InCmdList, groupX, groupY, 1);

    return true;
}

bool RCAS_Vk::DispatchDepthAdaptive(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                                    VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo,
                                    VkImageInfo* OutResourceInfo, VkImageInfo* InDepthInfo)
{
    (void) InDevice;

    if (InDepthInfo == VK_NULL_HANDLE || _pipelineDA == VK_NULL_HANDLE)
        return false;

    InternalConstantsDA constants {};

    constants.OutputWidth = OutResourceInfo->Width;
    constants.OutputHeight = OutResourceInfo->Height;
    constants.MotionWidth = InMotionVectorsInfo->Width;
    constants.MotionHeight = InMotionVectorsInfo->Height;
    constants.DepthWidth = InDepthInfo->Width;
    constants.DepthHeight = InDepthInfo->Height;

    FillMotionConstants(constants, InConstants);

    if (_mappedConstantBufferDA)
    {
        memcpy(_mappedConstantBufferDA, &constants, sizeof(InternalConstantsDA));
    }

    _currentSetIndex = (_currentSetIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    UpdateDescriptorSetDA(_descriptorSetsDA[_currentSetIndex], _constantBufferDA, InResourceInfo->ImageView,
                          InMotionVectorsInfo->ImageView, InDepthInfo->ImageView, OutResourceInfo->ImageView);

    vkCmdBindPipeline(InCmdList, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineDA);

    vkCmdBindDescriptorSets(InCmdList, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayoutDA, 0, 1,
                            &_descriptorSetsDA[_currentSetIndex], 0, nullptr);

    uint32_t groupX = (constants.OutputWidth + 15) / 16;
    uint32_t groupY = (constants.OutputHeight + 15) / 16;
    vkCmdDispatch(InCmdList, groupX, groupY, 1);

    return true;
}

bool RCAS_Vk::DispatchDASDepthAdaptive(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                                       VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo,
                                       VkImageInfo* OutResourceInfo, VkImageInfo* InDepthInfo)
{
    (void) InDevice;

    if (InDepthInfo == VK_NULL_HANDLE || _pipelineDASDA == VK_NULL_HANDLE)
        return false;

    InternalConstantsDA constants {};

    constants.OutputWidth = OutResourceInfo->Width;
    constants.OutputHeight = OutResourceInfo->Height;
    constants.MotionWidth = InMotionVectorsInfo->Width;
    constants.MotionHeight = InMotionVectorsInfo->Height;
    constants.DepthWidth = InDepthInfo->Width;
    constants.DepthHeight = InDepthInfo->Height;

    FillMotionConstants(constants, InConstants);

    if (_mappedConstantBufferDASDA)
    {
        memcpy(_mappedConstantBufferDASDA, &constants, sizeof(InternalConstantsDA));
    }

    _currentSetIndex = (_currentSetIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    UpdateDescriptorSetDA(_descriptorSetsDASDA[_currentSetIndex], _constantBufferDASDA, InResourceInfo->ImageView,
                          InMotionVectorsInfo->ImageView, InDepthInfo->ImageView, OutResourceInfo->ImageView);

    vkCmdBindPipeline(InCmdList, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineDASDA);

    vkCmdBindDescriptorSets(InCmdList, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayoutDASDA, 0, 1,
                            &_descriptorSetsDASDA[_currentSetIndex], 0, nullptr);

    uint32_t groupX = (constants.OutputWidth + 15) / 16;
    uint32_t groupY = (constants.OutputHeight + 15) / 16;
    vkCmdDispatch(InCmdList, groupX, groupY, 1);

    return true;
}

bool RCAS_Vk::Dispatch(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                       VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo, VkImageInfo* OutResourceInfo,
                       VkImageInfo* InDepthInfo)
{
    if (!_init || InDevice == VK_NULL_HANDLE || InCmdList == VK_NULL_HANDLE ||
        State::Instance().currentFeature == nullptr)
        return false;

    auto sharpnessShader = Config::Instance()->SharpnessShader.value_or_default();

    if (sharpnessShader == SharpenShader::LocalContrastDepthAware)
    {
        return DispatchDASDepthAdaptive(InDevice, InCmdList, InConstants, InResourceInfo, InMotionVectorsInfo,
                                        OutResourceInfo, InDepthInfo);
    }
    else if (sharpnessShader == SharpenShader::DepthAware)
    {
        return DispatchDepthAdaptive(InDevice, InCmdList, InConstants, InResourceInfo, InMotionVectorsInfo,
                                     OutResourceInfo, InDepthInfo);
    }
    else if (sharpnessShader == SharpenShader::RCAS)
    {
        return DispatchRCAS(InDevice, InCmdList, InConstants, InResourceInfo, InMotionVectorsInfo, OutResourceInfo);
    }
    else
    {
        return false;
    }
}

bool RCAS_Vk::CreateBufferResource(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer* buffer,
                                   VkDeviceMemory* memory, VkDeviceSize size, VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags properties)
{
    return Shader_Vk::CreateBufferResource(device, physicalDevice, buffer, memory, size, usage, properties);
}

void RCAS_Vk::SetBufferState(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize size, VkAccessFlags srcAccess,
                             VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
    Shader_Vk::SetBufferState(commandBuffer, buffer, size, srcAccess, dstAccess, srcStage, dstStage);
}

static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0;
}

bool RCAS_Vk::CreateImageResource(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
                                  VkFormat format, VkImageUsageFlags usage)
{
    if (_intermediateImage != VK_NULL_HANDLE && _width == width && _height == height && _format == format)
        return true;

    _width = width;
    _height = height;
    _format = format;

    ReleaseImageResource();

    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, &_intermediateImage) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create image!");
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, _intermediateImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &_intermediateMemory) != VK_SUCCESS)
    {
        LOG_ERROR("failed to allocate image memory!");
        return false;
    }

    vkBindImageMemory(device, _intermediateImage, _intermediateMemory, 0);

    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = _intermediateImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &_intermediateImageView) != VK_SUCCESS)
    {
        LOG_ERROR("failed to create image view!");
        return false;
    }

    return true;
}

void RCAS_Vk::ReleaseImageResource()
{
    if (_intermediateImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(_device, _intermediateImageView, nullptr);
        _intermediateImageView = VK_NULL_HANDLE;
    }

    if (_intermediateImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(_device, _intermediateImage, nullptr);
        _intermediateImage = VK_NULL_HANDLE;
    }

    if (_intermediateMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(_device, _intermediateMemory, nullptr);
        _intermediateMemory = VK_NULL_HANDLE;
    }
}

void RCAS_Vk::SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                             VkImageSubresourceRange subresourceRange)
{
    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    // Basic setting, might need refinement based on exact usage
    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        barrier.srcAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void RCAS_Vk::CreateConstantBuffer()
{
    VkDeviceSize bufferSize = sizeof(InternalConstants);

    // Create buffer using Shader_Vk helper
    if (!Shader_Vk::CreateBufferResource(_device, _physicalDevice, &_constantBuffer, &_constantBufferMemory, bufferSize,
                                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        LOG_ERROR("Failed to create constant buffer!");
        return;
    }

    vkMapMemory(_device, _constantBufferMemory, 0, bufferSize, 0, &_mappedConstantBuffer);
}

void RCAS_Vk::CreateConstantBufferDA(VkBuffer& constantBuffer, VkDeviceMemory& constantBufferMemory,
                                     void* mappedConstantBuffer)
{
    VkDeviceSize bufferSize = sizeof(InternalConstantsDA);

    if (!Shader_Vk::CreateBufferResource(_device, _physicalDevice, &constantBuffer, &constantBufferMemory, bufferSize,
                                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        LOG_ERROR("Failed to create depth adaptive constant buffer!");
        return;
    }

    vkMapMemory(_device, constantBufferMemory, 0, bufferSize, 0, &mappedConstantBuffer);
}
