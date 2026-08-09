#include "pch.h"
#include "Shader_Vk.h"
#include "Util.h"

Shader_Vk::Shader_Vk(std::string InName, VkDevice InDevice, VkPhysicalDevice InPhysicalDevice)
    : _name(InName), _device(InDevice), _physicalDevice(InPhysicalDevice)
{
}

Shader_Vk::~Shader_Vk()
{
    if (_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(_device, _pipeline, nullptr);
        _pipeline = VK_NULL_HANDLE;
    }
}

uint32_t Shader_Vk::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                   VkMemoryPropertyFlags properties)
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

    LOG_ERROR("Failed to find suitable memory type!");
    return -1;
}

bool Shader_Vk::CreateComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkPipeline* pipeline,
                                      const std::vector<char>& shaderCode, const char* entryPoint)
{
    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create shader module!");
        return false;
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = entryPoint;

    VkComputePipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create compute pipeline!");
        vkDestroyShaderModule(device, shaderModule, nullptr);
        return false;
    }

    vkDestroyShaderModule(device, shaderModule, nullptr);
    return true;
}

bool Shader_Vk::CreateBufferResource(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer* buffer,
                                     VkDeviceMemory* memory, VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties)
{
    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, *buffer, nullptr);
        *buffer = VK_NULL_HANDLE;
    }

    if (*memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, *memory, nullptr);
        *memory = VK_NULL_HANDLE;
    }

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create buffer!");
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, memory) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to allocate buffer memory!");
        return false;
    }

    vkBindBufferMemory(device, *buffer, *memory, 0);

    LOG_DEBUG("Created buffer size: {}", size);
    return true;
}

void Shader_Vk::SetBufferState(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize size,
                               VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage,
                               VkPipelineStageFlags dstStage)
{
    VkBufferMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = size;

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}
