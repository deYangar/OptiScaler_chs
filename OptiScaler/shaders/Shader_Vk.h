#pragma once

#include "SysUtils.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

// Matches NVSDK_NGX_ImageViewInfo_VK
struct VkImageInfo
{
    VkImageView ImageView;
    VkImage Image;
    VkImageSubresourceRange SubresourceRange;
    VkFormat Format;
    unsigned int Width;
    unsigned int Height;
};

class Shader_Vk
{
  protected:
    std::string _name = "";
    bool _init = false;

    VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

    VkPipeline _pipeline = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;

    static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                   VkMemoryPropertyFlags properties);
    static bool CreateComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkPipeline* pipeline,
                                      const std::vector<char>& shaderCode, const char* entryPoint = "CSMain");
    static bool CreateBufferResource(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer* buffer,
                                     VkDeviceMemory* memory, VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties);
    static void SetBufferState(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize size,
                               VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage,
                               VkPipelineStageFlags dstStage);

  public:
    bool IsInit() const { return _init; }

    Shader_Vk(std::string InName, VkDevice InDevice, VkPhysicalDevice InPhysicalDevice);
    virtual ~Shader_Vk();
};
