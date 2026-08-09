#pragma once
#pragma once

#include "SysUtils.h"
#include <shaders/Shader_Vk.h>
#include "RCAS_Common.h"

class RCAS_Vk : public Shader_Vk, public RCAS_Common
{
  public:
    RCAS_Vk(std::string InName, VkDevice InDevice, VkPhysicalDevice InPhysicalDevice);
    ~RCAS_Vk();

    bool Dispatch(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants, VkImageInfo* InResourceView,
                  VkImageInfo* InMotionVectorsInfo, VkImageInfo* OutResourceInfo, VkImageInfo* InDepthInfo = nullptr);

    bool CreateBufferResource(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer* buffer,
                              VkDeviceMemory* memory, VkDeviceSize size, VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties);
    void SetBufferState(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize size, VkAccessFlags srcAccess,
                        VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

    bool CreateImageResource(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
                             VkFormat format, VkImageUsageFlags usage);
    void ReleaseImageResource();
    void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                        VkImageSubresourceRange subresourceRange);

    VkImageView GetImageView() const { return _intermediateImageView; }
    VkImage GetImage() const { return _intermediateImage; }

    bool CanRender() const { return _init && _pipeline != VK_NULL_HANDLE; }

  private:
    VkBuffer _constantBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _constantBufferMemory = VK_NULL_HANDLE;
    VkBuffer _constantBufferDA = VK_NULL_HANDLE;
    VkBuffer _constantBufferDASDA = VK_NULL_HANDLE;
    VkDeviceMemory _constantBufferMemoryDA = VK_NULL_HANDLE;
    VkDeviceMemory _constantBufferMemoryDASDA = VK_NULL_HANDLE;
    VkSampler _nearestSampler = VK_NULL_HANDLE;
    void* _mappedConstantBuffer = nullptr;
    void* _mappedConstantBufferDA = nullptr;
    void* _mappedConstantBufferDASDA = nullptr;

    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    VkDescriptorPool _descriptorPoolDA = VK_NULL_HANDLE;
    VkDescriptorPool _descriptorPoolDASDA = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> _descriptorSets;
    std::vector<VkDescriptorSet> _descriptorSetsDA;
    std::vector<VkDescriptorSet> _descriptorSetsDASDA;
    uint32_t _currentSetIndex = 0;
    static const int MAX_FRAMES_IN_FLIGHT = 3;

    void CreateDescriptorSetLayout();
    void CreateDescriptorSetLayoutDA(VkDescriptorSetLayout& descriptorSetLayout, VkPipelineLayout& pipelineLayout);
    void CreateDescriptorPool();
    void CreateDescriptorPoolDA(VkDescriptorPool& descriptorPool);
    void CreateDescriptorSets();
    void CreateDescriptorSetsDA(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool,
                                std::vector<VkDescriptorSet>& descriptorSets);
    void CreateConstantBuffer();
    void CreateConstantBufferDA(VkBuffer& constantBuffer, VkDeviceMemory& constantBufferMemory,
                                void* mappedConstantBuffer);
    void UpdateDescriptorSet(VkCommandBuffer cmdList, int setIndex, VkImageView inputView, VkImageView motionView,
                             VkImageView outputView);
    void UpdateDescriptorSetDA(VkDescriptorSet descriptorSet, VkBuffer constantBuffer, VkImageView inputView,
                               VkImageView motionView, VkImageView depthView, VkImageView outputView);
    bool DispatchRCAS(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                      VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo, VkImageInfo* OutResourceInfo);
    bool DispatchDepthAdaptive(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                               VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo,
                               VkImageInfo* OutResourceInfo, VkImageInfo* InDepthInfo);
    bool DispatchDASDepthAdaptive(VkDevice InDevice, VkCommandBuffer InCmdList, RcasConstants InConstants,
                                  VkImageInfo* InResourceInfo, VkImageInfo* InMotionVectorsInfo,
                                  VkImageInfo* OutResourceInfo, VkImageInfo* InDepthInfo);

    VkImageView _intermediateImageView = VK_NULL_HANDLE;
    VkImage _intermediateImage = VK_NULL_HANDLE;
    VkDeviceMemory _intermediateMemory = VK_NULL_HANDLE;
    uint32_t _width = 0;
    uint32_t _height = 0;
    VkFormat _format = VK_FORMAT_UNDEFINED;

    VkPipeline _pipelineDA = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayoutDA = VK_NULL_HANDLE;
    VkDescriptorSetLayout _descriptorSetLayoutDA = VK_NULL_HANDLE;

    VkPipeline _pipelineDASDA = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayoutDASDA = VK_NULL_HANDLE;
    VkDescriptorSetLayout _descriptorSetLayoutDASDA = VK_NULL_HANDLE;
};
