#define STB_IMAGE_IMPLEMENTATION
#include "image.hpp"

#include "application.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "stb_image.h"

static uint32_t GetVulkanMemoryType(VkMemoryPropertyFlags properties,
                                    uint32_t type_bits) {
    VkPhysicalDeviceMemoryProperties prop;
    vkGetPhysicalDeviceMemoryProperties(Application::GetPhysicalDevice(),
                                        &prop);
    for (uint32_t i = 0; i < prop.memoryTypeCount; i++) {
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties &&
            type_bits & (1 << i))
            return i;
    }

    return 0xffffffff;
}

static VkFormat FormatToVulkanFormat(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGBA:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    return (VkFormat)0;
}

static uint32_t BytesPerPixel(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGBA:
            return 4;
        case ImageFormat::RGBA32F:
            return 16;
    }
    return 0;
}

Image::Image(uint32_t width, uint32_t height, ImageFormat format,
             const void* data)
    : _width(width), _height(height), _format(format) {
    AllocateMemory(_width * _height * BytesPerPixel(_format));
    if (data) SendData(data);
}

Image::~Image() { Release(); }

void Image::Release() {
    Application::SubmitResourceFree(
        [sampler = _sampler, imageView = _imageView, image = _image,
         memory = _memory, stagingBuffer = _stagingBuffer,
         stagingBufferMemory = _stagingBufferMemory]() {
            VkDevice device = Application::GetDevice();

            vkDestroySampler(device, sampler, nullptr);
            vkDestroyImageView(device, imageView, nullptr);
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, memory, nullptr);
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        });

    _sampler = nullptr;
    _imageView = nullptr;
    _image = nullptr;
    _memory = nullptr;
    _stagingBuffer = nullptr;
    _stagingBufferMemory = nullptr;
}

void Image::AllocateMemory(uint64_t size) {
    {
        printf("Allocating memory for image...\n");
        VkDevice device = Application::GetDevice();

        VkResult err;

        VkFormat vulkanFormat = FormatToVulkanFormat(_format);

        // Create the Image
        {
            VkImageCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.imageType = VK_IMAGE_TYPE_2D;
            info.format = vulkanFormat;
            info.extent.width = _width;
            info.extent.height = _height;
            info.extent.depth = 1;
            info.mipLevels = 1;
            info.arrayLayers = 1;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage =
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            err = vkCreateImage(device, &info, nullptr, &_image);
            check_vk_result(err);
            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(device, _image, &req);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = GetVulkanMemoryType(
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
            err = vkAllocateMemory(device, &alloc_info, nullptr, &_memory);
            check_vk_result(err);
            err = vkBindImageMemory(device, _image, _memory, 0);
            check_vk_result(err);
        }

        // Create the Image View:
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = _image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = vulkanFormat;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.layerCount = 1;
            err = vkCreateImageView(device, &info, nullptr, &_imageView);
            check_vk_result(err);
        }

        // Create sampler:
        {
            VkSamplerCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            info.magFilter = VK_FILTER_LINEAR;
            info.minFilter = VK_FILTER_LINEAR;
            info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.minLod = -1000;
            info.maxLod = 1000;
            info.maxAnisotropy = 1.0f;
            VkResult err = vkCreateSampler(device, &info, nullptr, &_sampler);
            check_vk_result(err);
        }

        // Create the Descriptor Set:
        _descriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(
            _sampler, _imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    printf("Created image with descriptor set: %p\n", _descriptorSet);
}

void Image::SendData(const void* data) {
    VkDevice device = Application::GetDevice();

    size_t upload_size = _width * _height * BytesPerPixel(_format);

    VkResult err;

    if (!_stagingBuffer) {
        // Create the Upload Buffer
        {
            VkBufferCreateInfo buffer_info = {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = upload_size;
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            err =
                vkCreateBuffer(device, &buffer_info, nullptr, &_stagingBuffer);
            check_vk_result(err);
            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(device, _stagingBuffer, &req);
            _alignedSize = req.size;
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = GetVulkanMemoryType(
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
            err = vkAllocateMemory(device, &alloc_info, nullptr,
                                   &_stagingBufferMemory);
            check_vk_result(err);
            err = vkBindBufferMemory(device, _stagingBuffer,
                                     _stagingBufferMemory, 0);
            check_vk_result(err);
        }
    }

    // Upload to Buffer
    {
        char* map = NULL;
        err = vkMapMemory(device, _stagingBufferMemory, 0, _alignedSize, 0,
                          (void**)(&map));
        check_vk_result(err);
        memcpy(map, data, upload_size);
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = _stagingBufferMemory;
        range[0].size = _alignedSize;
        err = vkFlushMappedMemoryRanges(device, 1, range);
        check_vk_result(err);
        vkUnmapMemory(device, _stagingBufferMemory);
    }

    // Copy to Image
    {
        VkCommandBuffer command_buffer = Application::GetCommandBuffer(true);

        VkImageMemoryBarrier copy_barrier = {};
        copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.image = _image;
        copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier.subresourceRange.levelCount = 1;
        copy_barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                             NULL, 1, &copy_barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = _width;
        region.imageExtent.height = _height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(command_buffer, _stagingBuffer, _image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);

        VkImageMemoryBarrier use_barrier = {};
        use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.image = _image;
        use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier.subresourceRange.levelCount = 1;
        use_barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL,
                             0, NULL, 1, &use_barrier);

        Application::FlushCommandBuffer(command_buffer);
    }
}