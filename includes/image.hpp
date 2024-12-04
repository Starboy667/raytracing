#pragma once

#include "vulkan/vulkan.h"

enum class ImageFormat { None = 0, RGBA, RGBA32F };

class Image {
   public:
    Image(uint32_t width, uint32_t height, ImageFormat format,
          const void* data = nullptr);
    ~Image();
    uint32_t GetWidth() const { return _width; }
    uint32_t GetHeight() const { return _height; }
    VkDescriptorSet GetDescriptorSet() const { return _descriptorSet; }
    void Image::SendData(const void* data);

   private:
    void AllocateMemory(uint64_t size);
    void Release();

   private:
    uint32_t _width = 0;
    uint32_t _height = 0;
    VkImage _image = nullptr;
    VkDeviceMemory _memory = nullptr;
    VkImageView _imageView = nullptr;
    VkSampler _sampler = nullptr;
    VkDescriptorSet _descriptorSet = nullptr;
    VkBuffer _stagingBuffer = nullptr;
    VkDeviceMemory _stagingBufferMemory = nullptr;
    size_t _alignedSize = 0;
    ImageFormat _format = ImageFormat::None;
};