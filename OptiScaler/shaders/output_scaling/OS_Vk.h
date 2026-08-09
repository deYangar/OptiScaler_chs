#pragma once

#include "SysUtils.h"

#include <shaders/Shader_Vk.h>

class OS_Vk : public Shader_Vk
{
  public:
    OS_Vk(std::string InName, VkDevice InDevice, VkPhysicalDevice InPhysicalDevice, bool InUpsample);
    ~OS_Vk();

    bool Dispatch(VkDevice InDevice, VkCommandBuffer InCmdList, VkImageView InResourceView, VkImageView OutResourceView,
                  VkExtent2D OutExtent);

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
    VkSampler _textureSampler = VK_NULL_HANDLE;
    void* _mappedConstantBuffer = nullptr;
    bool _upsample = false;

    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> _descriptorSets;
    uint32_t _currentSetIndex = 0;
    static const int MAX_FRAMES_IN_FLIGHT = 3;

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateConstantBuffer();
    void UpdateDescriptorSet(VkCommandBuffer cmdList, int setIndex, VkImageView inputView, VkImageView outputView);

    VkImageView _intermediateImageView = VK_NULL_HANDLE;
    VkImage _intermediateImage = VK_NULL_HANDLE;
    VkDeviceMemory _intermediateMemory = VK_NULL_HANDLE;
    uint32_t _width = 0;
    uint32_t _height = 0;
    VkFormat _format = VK_FORMAT_UNDEFINED;
};
