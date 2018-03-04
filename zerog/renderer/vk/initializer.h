#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <cstdint>
#include <cstdlib>

#include "vulkan/vulkan.h"

#include "memory/memory.h"
#include "renderer/types.h"

typedef struct GLFWwindow *VkWindow;

namespace ZeroG {

ARRAY_DEFINITION(vk_image_array, VkImage);
ARRAY_DEFINITION(vk_image_view_array, VkImageView);

struct InstanceExt {
  VkInstance instance;
  VkDebugReportCallbackEXT debug;
};

struct QueueFamilyIndices {
  int32_t graphics;
  int32_t present;
};

struct PhysicalDeviceExt {
  VkPhysicalDevice device;
  QueueFamilyIndices indics;
};

struct LogicalDeviceExt {
  VkDevice device;
  VkQueue graphics;
  VkQueue present;
};

struct Kernel {
  VkWindow window;
  InstanceExt instance;
  VkSurfaceKHR surface;
  PhysicalDeviceExt physical_device;
  LogicalDeviceExt logical_device;
};

enum PresentMode { IMMEDIATE = 0, MAILBOX = 1, FIFO = 2, FIFO_RELAXED = 3 };

struct SwapChainProperties {
  VkExtent2D extent;
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR format;
  VkPresentModeKHR presentMode;
};

struct SwapChainExt {
  VkSwapchainKHR swap_chain;
  SwapChainProperties properties;
};

struct SwapChainImagesExt {
  vk_image_array images;
  vk_image_view_array views;
};

struct PipelineExt {
  VkPipeline pipeline;
  VkPipelineLayout pipe_layout;
  VkDescriptorSetLayout desc_set_layout;
};

namespace vk {
VkWindow create_window(const WindowCreateInfo *info);
void destroy_window(VkWindow window);

InstanceExt create_instance(allocator *alloc, const AppCreateInfo *info);
void destroy_instance(InstanceExt instance);

VkSurfaceKHR create_surface(VkInstance instance, VkWindow window);
void destroy_surface(VkInstance instance, VkSurfaceKHR surface);

PhysicalDeviceExt select_physical_device(allocator *alloc, VkInstance instance,
                                         VkSurfaceKHR surface);

LogicalDeviceExt create_device(allocator *alloc, PhysicalDeviceExt device);
void destroy_device(LogicalDeviceExt device);

SwapChainExt create_swap_chain(allocator *alloc, const Kernel *kernel,
                               VkPresentModeKHR prefered_present_mode =
                                   VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR);

void destroy_swap_chain(const SwapChainExt *swap_chain, const Kernel *kernel);

SwapChainImagesExt create_swap_chain_images(allocator *alloc,
                                            const Kernel *kernel,
                                            const SwapChainExt *swap_chain);

void destroy_swap_chain_images(allocator *alloc, const Kernel *kernel,
                               const SwapChainImagesExt *images);

VkRenderPass create_render_pass_color_depth(const Kernel *kernel,
                                            const SwapChainExt *swap_chain);

void destroy_render_pass(const Kernel *kernel, VkRenderPass render_pass);

PipelineExt create_graphics_pipeline(
    allocator *alloc, const Kernel *kernel, const SwapChainExt *swapchain,
    VkRenderPass render_pass, const PipelineCreateInfo *pipeline_layout_info);

void destroy_graphics_pipeline(const Kernel *kernel, PipelineExt pipeline);

VkCommandPool create_graphics_commad_pool(const Kernel *kernel);

void destroy_graphics_command_pool(const Kernel *kernel,
                                   VkCommandPool cmd_pool);

namespace util {
VkImageView create_image_view(const Kernel *kernel, VkImage image,
                              VkFormat format, VkImageAspectFlags aspectFlags);
void destroy_image_view(const Kernel *kernel, VkImageView view);

VkDescriptorSetLayout
create_descriptor_set_layout(allocator *alloc, const Kernel *kernel,
                             const DescriptorSetCreateInfo *desc_info);
void destroy_descriptor_set_layout(const Kernel *kernel,
                                   VkDescriptorSetLayout desc_layout);

VkPipelineLayout create_pipeline_layout(const Kernel *kernel,
                                        VkDescriptorSetLayout desc_layuout);
void destroy_pipeline_layout(const Kernel *kernel,
                             VkPipelineLayout pipe_layout);
VkShaderModule create_shader_module(const Kernel *kenel, uint32_t *code,
                                    size_t length);
void destroy_shader_module(const Kernel *kernel, VkShaderModule module);
} // namespace util

} // namespace vk
} // namespace ZeroG

#endif // INITIALIZER_H
