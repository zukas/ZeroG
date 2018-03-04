#include "initializer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <limits>

#include "common/math.h"

#ifndef NDEBUG
#include <cstdio>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t, size_t,
    int32_t code, const char *layerPrefix, const char *msg, void *) {

  if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    printf("VINFO: [%s] Code %d : %s\n", layerPrefix, code, msg);
  } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    printf("VWARNING: [%s] Code %d : %s\n", layerPrefix, code, msg);

  } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    printf("VPERF: [%s] Code %d : %s\n", layerPrefix, code, msg);

  } else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    printf("VERROR: [%s] Code %d : %s\n", layerPrefix, code, msg);

  } else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    printf("VDEBUG: [%s] Code %d : %s\n", layerPrefix, code, msg);

  } else {
    printf("VINFO: [%s] Code %d : %s\n", layerPrefix, code, msg);
  }

  fflush(stdout);
  return VK_FALSE;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback) {
  auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance instance, VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks *pAllocator) {
  auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

constexpr const char *validation_layers[]{
    "VK_LAYER_LUNARG_standard_validation"};

constexpr int32_t validation_layer_count{sizeof(validation_layers) /
                                         sizeof(const char *)};
#endif

constexpr const char *device_extensions[]{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr int32_t device_extension_count{sizeof(device_extensions) /
                                         sizeof(const char *)};

ARRAY_DEFINITION(string_array, const char *);
ARRAY_DEFINITION(vk_lp_array, VkLayerProperties);
ARRAY_DEFINITION(vk_qfp_array, VkQueueFamilyProperties);
ARRAY_DEFINITION(vk_ep_array, VkExtensionProperties);
ARRAY_DEFINITION(vk_pd_array, VkPhysicalDevice);
ARRAY_DEFINITION(vk_format_array, VkFormat);
ARRAY_DEFINITION(vk_dsl_array, VkDescriptorSetLayoutBinding);
ARRAY_DEFINITION(vk_pss_array, VkPipelineShaderStageCreateInfo);
ARRAY_DEFINITION(vk_vibd_array, VkVertexInputBindingDescription);
ARRAY_DEFINITION(vk_viad_array, VkVertexInputAttributeDescription);

ARRAY_IMPLEMENTATION(string_array);
ARRAY_IMPLEMENTATION(vk_lp_array);
ARRAY_IMPLEMENTATION(vk_qfp_array);
ARRAY_IMPLEMENTATION(vk_ep_array);
ARRAY_IMPLEMENTATION(vk_pd_array);
ARRAY_IMPLEMENTATION(vk_format_array);
ARRAY_IMPLEMENTATION(vk_dsl_array);
ARRAY_IMPLEMENTATION(vk_pss_array);
ARRAY_IMPLEMENTATION(vk_vibd_array);
ARRAY_IMPLEMENTATION(vk_viad_array);

void print_string_array(const string_array &a) {
  for (const auto &m : a) {
    printf("val: %s\n", m);
  }
}

static string_array get_extensions(allocator *alloc);
static string_array get_validation_layers(allocator *alloc);
static int32_t rate_physical_device(allocator *alloc, VkPhysicalDevice device,
                                    VkSurfaceKHR surface,
                                    ZeroG::QueueFamilyIndices indices);
static ZeroG::QueueFamilyIndices
get_queue_family_indeces(allocator *alloc, VkPhysicalDevice device,
                         VkSurfaceKHR surface);
static bool verify_extensions(allocator *alloc, VkPhysicalDevice device);
static bool verify_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface);

static string_array get_extensions(allocator *alloc) {
  uint32_t extesion_count = 0;
  const char **extension_names;
  extension_names = glfwGetRequiredInstanceExtensions(&extesion_count);

  auto res = create_string_array(alloc, extesion_count + 1);
  int32_t ext_count = static_cast<int32_t>(extesion_count);

  for (int32_t i = 0; i < ext_count; i++) {
    res.data[i] = extension_names[i];
  }

#ifndef NDEBUG
  res.data[extesion_count] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
#endif
  return res;
}

#ifndef NDEBUG
static string_array get_validation_layers(allocator *alloc) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  auto res = create_string_array(alloc, validation_layer_count);
  auto layers = create_vk_lp_array(alloc, layer_count);

  vkEnumerateInstanceLayerProperties(&layer_count, layers.data);

  int32_t i;
  for (i = 0; i < validation_layer_count; i++) {
    bool layer_found{false};
    size_t j;
    for (j = 0; j < layer_count; j++) {
      if (strcmp(validation_layers[i], layers.data[j].layerName) == 0) {
        res.data[i] = validation_layers[i];
        layer_found = true;
        break;
      }
    }
    if (j == layer_count) {
      break;
    }
  }

  destroy_vk_lp_array(alloc, layers);

  if (i != validation_layer_count) {
    res = {nullptr, 0, 0};
  }

  return res;
}
#else
static string_array get_validation_layers(allocator *) {
  return {nullptr, 0, 0};
}
#endif

static int32_t rate_physical_device(allocator *alloc, VkPhysicalDevice device,
                                    VkSurfaceKHR surface,
                                    ZeroG::QueueFamilyIndices indices) {
  int32_t score = 0;

  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;

  vkGetPhysicalDeviceFeatures(device, &features);
  vkGetPhysicalDeviceProperties(device, &properties);

  if (features.geometryShader &&
      (indices.graphics >= 0 && indices.present >= 0) &&
      verify_extensions(alloc, device) && verify_swapchain(device, surface) &&
      features.samplerAnisotropy) {
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      score = 1000;

    score += properties.limits.maxColorAttachments;
  }

  return score;
}

static ZeroG::QueueFamilyIndices
get_queue_family_indeces(allocator *alloc, VkPhysicalDevice device,
                         VkSurfaceKHR surface) {

  ZeroG::QueueFamilyIndices result{-1, -1};

  uint32_t family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);

  auto properties = create_vk_qfp_array(alloc, family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count,
                                           properties.data);

  int32_t fm_count = static_cast<int32_t>(family_count);

  for (int32_t i = 0; i < fm_count; i++) {
    if (properties.data[i].queueCount > 0 &&
        properties.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      result.graphics = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i),
                                         surface, &present_support);
    if (properties.data[i].queueCount > 0 && present_support) {
      result.present = i;
    }
    if (result.graphics >= 0 && result.present >= 0)
      break;
  }

  destroy_vk_qfp_array(alloc, properties);

  return result;
}

static bool verify_extensions(allocator *alloc, VkPhysicalDevice device) {
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);

  auto extensions = create_vk_ep_array(alloc, extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       extensions.data);
  int32_t found = 0;
  for (int32_t i = 0; i < device_extension_count; ++i) {
    for (auto ext : extensions) {
      if (strcmp(device_extensions[i], ext.extensionName) == 0) {
        ++found;
        break;
      }
    }
  }
  destroy_vk_ep_array(alloc, extensions);
  return found == device_extension_count;
}

static bool verify_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface) {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
  uint32_t mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count,
                                            nullptr);

  return capabilities.minImageCount > 0 &&
         capabilities.maxImageArrayLayers > 0 && format_count > 0 &&
         mode_count > 0;
}

static VkFormat selectSupportedFormat(VkPhysicalDevice device,
                                      vk_format_array formats,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features) {
  for (VkFormat format : formats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  return VK_FORMAT_UNDEFINED;
}

static VkFormat selectDepthFormat(VkPhysicalDevice device) {

  VkFormat formats[3]{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                      VK_FORMAT_D24_UNORM_S8_UINT};
  vk_format_array format_array;
  format_array.data = formats;
  format_array.count = 3;
  format_array.size = 0;
  return selectSupportedFormat(device, format_array, VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

namespace ZeroG {
ARRAY_IMPLEMENTATION(vk_image_array);
ARRAY_IMPLEMENTATION(vk_image_view_array);
namespace vk {

VkWindow create_window(const WindowCreateInfo *info) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  int32_t width{0};
  int32_t height{0};
  GLFWmonitor *monitor{nullptr};
  if (info->full_screen != 0) {
    monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    width = mode->width;
    height = mode->height;
  } else {
    width = info->width;
    height = info->height;
  }
  return glfwCreateWindow(width, height, info->caption, monitor, nullptr);
}

void destroy_window(VkWindow window) {
  glfwDestroyWindow(window);
  glfwTerminate();
}

ZeroG::InstanceExt create_instance(allocator *alloc,
                                   const AppCreateInfo *info) {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = info->app_name;
  app_info.applicationVersion = info->app_version;
  app_info.pEngineName = info->engine_name;
  app_info.engineVersion = info->engien_version;
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  auto extensions = get_extensions(alloc);
  instance_info.ppEnabledExtensionNames = extensions.data;
  instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.count);

  auto val_layers = get_validation_layers(alloc);
  instance_info.ppEnabledLayerNames = val_layers.data;
  instance_info.enabledLayerCount = static_cast<uint32_t>(val_layers.count);

  printf("Extensions\n");
  print_string_array(extensions);
  printf("Validation Layers\n");
  print_string_array(val_layers);

#ifndef NDEBUG
  VkDebugReportCallbackCreateInfoEXT debug_create_info{};
  debug_create_info.sType =
      VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  debug_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                            VK_DEBUG_REPORT_WARNING_BIT_EXT |
                            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                            VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
  debug_create_info.pfnCallback = debug_callback;
  debug_create_info.pUserData = nullptr;
  debug_create_info.pNext = nullptr;
  instance_info.pNext = &debug_create_info;
#endif

  VkInstance instance;
  auto r = vkCreateInstance(&instance_info, nullptr, &instance);
  VkDebugReportCallbackEXT debug{VK_NULL_HANDLE};
  if (r == VK_SUCCESS) {
#ifndef NDEBUG
    vkCreateDebugReportCallbackEXT(instance, &debug_create_info, nullptr,
                                   &debug);
#endif
  }

  if (val_layers.data)
    destroy_string_array(alloc, val_layers);
  destroy_string_array(alloc, extensions);

  return {instance, debug};
}

void destroy_instance(InstanceExt instance) {
#ifndef NDEBUG
  if (instance.debug)
    vkDestroyDebugReportCallbackEXT(instance.instance, instance.debug, nullptr);
#endif
  vkDestroyInstance(instance.instance, nullptr);
}

VkSurfaceKHR create_surface(VkInstance instance, VkWindow window) {
  VkSurfaceKHR surface;
  glfwCreateWindowSurface(instance, window, nullptr, &surface);
  return surface;
}

void destroy_surface(VkInstance instance, VkSurfaceKHR surface) {
  vkDestroySurfaceKHR(instance, surface, nullptr);
}

ZeroG::PhysicalDeviceExt select_physical_device(allocator *alloc,
                                                VkInstance instance,
                                                VkSurfaceKHR surface) {
  PhysicalDeviceExt result{VK_NULL_HANDLE, {-1, -1}};
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  auto devices = create_vk_pd_array(alloc, device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data);

  int32_t max_score = 0;
  for (auto dev : devices) {
    auto indices = get_queue_family_indeces(alloc, dev, surface);
    if (indices.graphics >= 0 && indices.present >= 0) {
      int32_t score = rate_physical_device(alloc, dev, surface, indices);
      if (score > max_score) {
        max_score = score;
        result = {dev, indices};
      }
    }
  }

  return result;
}

ZeroG::LogicalDeviceExt create_device(allocator *alloc,
                                      PhysicalDeviceExt device) {
  LogicalDeviceExt result;

  VkDeviceQueueCreateInfo queue_info[2]{};
  uint32_t queue_info_count{1};

  float queuePriority = 1.0f;
  queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info[0].queueFamilyIndex =
      static_cast<uint32_t>(device.indics.graphics);
  queue_info[0].queueCount = 1;
  queue_info[0].pQueuePriorities = &queuePriority;
  if (device.indics.graphics != device.indics.present) {
    queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[1].queueFamilyIndex =
        static_cast<uint32_t>(device.indics.present);
    queue_info[1].queueCount = 1;
    queue_info[1].pQueuePriorities = &queuePriority;
    queue_info_count = 2;
  }

  VkPhysicalDeviceFeatures features{};
  features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pQueueCreateInfos = queue_info;
  device_info.queueCreateInfoCount = queue_info_count;
  device_info.pEnabledFeatures = &features;
  device_info.enabledExtensionCount = device_extension_count;
  device_info.ppEnabledExtensionNames = device_extensions;

  auto val_layers = get_validation_layers(alloc);
  device_info.ppEnabledLayerNames = val_layers.data;
  device_info.enabledLayerCount = static_cast<uint32_t>(val_layers.count);

  if (vkCreateDevice(device.device, &device_info, nullptr, &result.device) ==
      VK_SUCCESS) {
    vkGetDeviceQueue(result.device,
                     static_cast<uint32_t>(device.indics.graphics), 0,
                     &result.graphics);
    vkGetDeviceQueue(result.device,
                     static_cast<uint32_t>(device.indics.present), 0,
                     &result.present);
  }
  if (val_layers.data)
    destroy_string_array(alloc, val_layers);

  return result;
}

void destroy_device(LogicalDeviceExt device) {
  vkDestroyDevice(device.device, nullptr);
}

/**
 * USER SPACE FUNCTIONS
 */

ARRAY_DEFINITION(vk_sf_array, VkSurfaceFormatKHR);
ARRAY_DEFINITION(vk_pm_array, VkPresentModeKHR);

ARRAY_IMPLEMENTATION(vk_sf_array);
ARRAY_IMPLEMENTATION(vk_pm_array);

ZeroG::SwapChainExt create_swap_chain(allocator *alloc,
                                      const ZeroG::Kernel *kernel,
                                      VkPresentModeKHR prefered_present_mode) {

  SwapChainProperties properties;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(kernel->physical_device.device,
                                            kernel->surface,
                                            &properties.capabilities);
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(kernel->physical_device.device,
                                       kernel->surface, &format_count, nullptr);
  if (format_count > 0) {
    vk_sf_array formats = create_vk_sf_array(alloc, format_count);
    VkSurfaceFormatKHR format{VK_FORMAT_UNDEFINED,
                              VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    for (const auto &f : formats) {
      if ((f.format == VK_FORMAT_B8G8R8A8_UNORM &&
           f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) ||
          f.format == VK_FORMAT_UNDEFINED) {
        format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        break;
      }
    }
    if (format.format == VK_FORMAT_UNDEFINED)
      format = formats.data[0];
    properties.format = format;
    destroy_vk_sf_array(alloc, formats);
  }
  uint32_t mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      kernel->physical_device.device, kernel->surface, &mode_count, nullptr);
  if (mode_count > 0) {
    vk_pm_array modes = create_vk_pm_array(alloc, mode_count);
    VkPresentModeKHR mode{VK_PRESENT_MODE_FIFO_KHR};
    for (const auto &m : modes) {
      if (m == prefered_present_mode) {
        mode = m;
        break;
      }
    }
    properties.presentMode = mode;
  }

  int width, height;
  glfwGetWindowSize(kernel->window, &width, &height);
  if (properties.capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    properties.extent = properties.capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width = max(
        properties.capabilities.minImageExtent.width,
        min(properties.capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = max(properties.capabilities.minImageExtent.height,
                              min(properties.capabilities.maxImageExtent.height,
                                  actualExtent.height));

    properties.extent = actualExtent;
  }
  uint32_t image_count = properties.capabilities.minImageCount + 1;
  if (properties.capabilities.maxImageCount > 0 &&
      image_count > properties.capabilities.maxImageCount) {
    image_count = properties.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swap_chain_info{};
  swap_chain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_chain_info.surface = kernel->surface;
  swap_chain_info.minImageCount = image_count;
  swap_chain_info.imageFormat = properties.format.format;
  swap_chain_info.imageColorSpace = properties.format.colorSpace;
  swap_chain_info.imageExtent = properties.extent;
  swap_chain_info.imageArrayLayers = 1;
  swap_chain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t indices[] = {
      static_cast<uint32_t>(kernel->physical_device.indics.graphics),
      static_cast<uint32_t>(kernel->physical_device.indics.present)};
  if (indices[0] != indices[1]) {
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swap_chain_info.queueFamilyIndexCount = 2;
    swap_chain_info.pQueueFamilyIndices = indices;
  } else {
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_info.queueFamilyIndexCount = 1;
    swap_chain_info.pQueueFamilyIndices = indices;
  }
  swap_chain_info.preTransform = properties.capabilities.currentTransform;
  swap_chain_info.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // can this be changed?
  swap_chain_info.presentMode = properties.presentMode;
  swap_chain_info.clipped = VK_TRUE;
  swap_chain_info.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swap_chain;

  vkCreateSwapchainKHR(kernel->logical_device.device, &swap_chain_info, nullptr,
                       &swap_chain);

  return {swap_chain, properties};
}

void destroy_swap_chain(const SwapChainExt *swap_chain,
                        const ZeroG::Kernel *kernel) {
  vkDestroySwapchainKHR(kernel->logical_device.device, swap_chain->swap_chain,
                        nullptr);
}

ZeroG::SwapChainImagesExt
create_swap_chain_images(allocator *alloc, const ZeroG::Kernel *kernel,
                         const ZeroG::SwapChainExt *swap_chain) {

  uint32_t count = 0;
  vkGetSwapchainImagesKHR(kernel->logical_device.device, swap_chain->swap_chain,
                          &count, nullptr);
  vk_image_array images = create_vk_image_array(alloc, count);

  vkGetSwapchainImagesKHR(kernel->logical_device.device, swap_chain->swap_chain,
                          &count, images.data);

  vk_image_view_array views = create_vk_image_view_array(alloc, count);
  int32_t len = static_cast<int32_t>(count);
  for (int32_t i = 0; i < len; i++) {
    views.data[i] = util::create_image_view(
        kernel, images.data[i], swap_chain->properties.format.format,
        VK_IMAGE_ASPECT_COLOR_BIT);
  }

  return {images, views};
}

void destroy_swap_chain_images(allocator *alloc, const ZeroG::Kernel *kernel,
                               const ZeroG::SwapChainImagesExt *images) {

  for (const auto &v : images->views) {
    util::destroy_image_view(kernel, v);
  }
  destroy_vk_image_view_array(alloc, images->views);
  destroy_vk_image_array(alloc, images->images);
}

VkRenderPass create_render_pass_color_depth(const Kernel *kernel,
                                            const SwapChainExt *swap_chain) {
  VkAttachmentDescription attachments[2];
  attachments[0].format = swap_chain->properties.format.format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  attachments[1].format = selectDepthFormat(kernel->physical_device.device);
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref{};
  depth_attachment_ref.attachment = 1;
  depth_attachment_ref.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = &depth_attachment_ref;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount =
      sizeof(attachments) / sizeof(VkAttachmentDescription);
  render_pass_info.pAttachments = attachments;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.pDependencies = &dependency;
  render_pass_info.dependencyCount = 1;

  VkRenderPass render_pass{VK_NULL_HANDLE};
  vkCreateRenderPass(kernel->logical_device.device, &render_pass_info, nullptr,
                     &render_pass);
  return render_pass;
}

void destroy_render_pass(const Kernel *kernel, VkRenderPass render_pass) {
  vkDestroyRenderPass(kernel->logical_device.device, render_pass, nullptr);
}

PipelineExt create_graphics_pipeline(
    allocator *alloc, const Kernel *kernel, const SwapChainExt *swapchain,
    VkRenderPass render_pass, const PipelineCreateInfo *pipeline_layout_info) {
  vk_pss_array pss =
      create_vk_pss_array(alloc, pipeline_layout_info->shader_count);
  int32_t sc = static_cast<int32_t>(pipeline_layout_info->shader_count);
  for (int32_t i = 0; i < sc; i++) {
    pss.data[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pss.data[i].stage = static_cast<VkShaderStageFlagBits>(
        pipeline_layout_info->shaders[i].stage);
    pss.data[i].module = util::create_shader_module(
        kernel, pipeline_layout_info->shaders[i].code,
        pipeline_layout_info->shaders[i].length);
    pss.data[i].pName = "main";
  }

  DataLayoutCreateInfo *dlci = pipeline_layout_info->data_layout_info;
  int32_t bc = static_cast<int32_t>(dlci->binding_count);
  uint32_t ac = 0;
  for (int32_t i = 0; i < bc; i++) {
    ac += dlci->bindings[i].attr_count;
  }

  vk_vibd_array vibd = create_vk_vibd_array(alloc, dlci->binding_count);
  vk_viad_array viad = create_vk_viad_array(alloc, ac);

  for (int32_t i = 0, j = 0; i < bc; ++i) {

    DataBinding &binding = dlci->bindings[i];
    uint32_t binding_idx = static_cast<uint32_t>(i);

    vibd.data[i].binding = binding_idx;
    vibd.data[i].inputRate = static_cast<VkVertexInputRate>(binding.rate);
    vibd.data[i].stride = binding.stride;

    int32_t iac = static_cast<int32_t>(binding.attr_count);
    for (int32_t k = 0; k < iac; k++, j++) {
      viad.data[j].binding = binding_idx;
      viad.data[j].location = static_cast<uint32_t>(k);
      viad.data[j].format = static_cast<VkFormat>(binding.attributes[k].type);
      viad.data[j].offset = binding.attributes[k].offset;
    }
  }

  VkDescriptorSetLayout descriptor_set_layout =
      util::create_descriptor_set_layout(alloc, kernel,
                                         pipeline_layout_info->descriptor_info);

  VkPipelineLayout pipeline_layout =
      util::create_pipeline_layout(kernel, descriptor_set_layout);

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = dlci->binding_count;
  vertex_input_info.pVertexBindingDescriptions = vibd.data;
  vertex_input_info.vertexAttributeDescriptionCount = ac;
  vertex_input_info.pVertexAttributeDescriptions = viad.data;

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = swapchain->properties.extent.width;
  viewport.height = swapchain->properties.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapchain->properties.extent;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f;          // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;          // Optional
  multisampling.pSampleMask = nullptr;            // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE;      // Optional

  VkPipelineColorBlendAttachmentState cb_attachment{};
  cb_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  cb_attachment.blendEnable = VK_FALSE;
  cb_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  cb_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  cb_attachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
  cb_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  cb_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  cb_attachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

  VkPipelineColorBlendStateCreateInfo colour_blending{};
  colour_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colour_blending.logicOpEnable = VK_FALSE;
  colour_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colour_blending.attachmentCount = 1;
  colour_blending.pAttachments = &cb_attachment;
  colour_blending.blendConstants[0] = 0.0f; // Optional
  colour_blending.blendConstants[1] = 0.0f; // Optional
  colour_blending.blendConstants[2] = 0.0f; // Optional
  colour_blending.blendConstants[3] = 0.0f; // Optional

  VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_LINE_WIDTH};

  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = 2;
  dynamic_state.pDynamicStates = dynamic_states;

  VkPipelineDepthStencilStateCreateInfo depth_stencil{};
  depth_stencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthTestEnable = VK_TRUE;
  depth_stencil.depthWriteEnable = VK_TRUE;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.minDepthBounds = 0.0f; // Optional
  depth_stencil.maxDepthBounds = 1.0f; // Optional
  depth_stencil.stencilTestEnable = VK_FALSE;
  depth_stencil.front = {}; // Optional
  depth_stencil.back = {};  // Optional

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<uint32_t>(pss.count);
  pipeline_info.pStages = pss.data;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = nullptr; // Optional
  pipeline_info.pColorBlendState = &colour_blending;
  pipeline_info.pDynamicState = nullptr; // Optional
  pipeline_info.pDepthStencilState = &depth_stencil;
  pipeline_info.layout = pipeline_layout;
  pipeline_info.renderPass = render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeline_info.basePipelineIndex = -1;

  VkPipeline pipeline;
  vkCreateGraphicsPipelines(kernel->logical_device.device, VK_NULL_HANDLE, 1,
                            &pipeline_info, nullptr, &pipeline);

  destroy_vk_viad_array(alloc, viad);
  destroy_vk_vibd_array(alloc, vibd);
  destroy_vk_pss_array(alloc, pss);

  return {pipeline, pipeline_layout, descriptor_set_layout};
}

void destroy_graphics_pipeline(const Kernel *kernel, PipelineExt pipeline) {
  vkDestroyPipeline(kernel->logical_device.device, pipeline.pipeline, nullptr);
  util::destroy_pipeline_layout(kernel, pipeline.pipe_layout);
  util::destroy_descriptor_set_layout(kernel, pipeline.desc_set_layout);
}

VkCommandPool create_graphics_commad_pool(const Kernel *kernel) {
  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex =
      static_cast<uint32_t>(kernel->physical_device.indics.graphics);
  pool_info.flags = 0; // Optional

  VkCommandPool cmd_pool;
  vkCreateCommandPool(kernel->logical_device.device, &pool_info, nullptr,
                      &cmd_pool);
  return cmd_pool;
}

void destroy_graphics_command_pool(const Kernel *kernel,
                                   VkCommandPool cmd_pool) {
  vkDestroyCommandPool(kernel->logical_device.device, cmd_pool, nullptr);
}

namespace util {

VkImageView create_image_view(const ZeroG::Kernel *kernel, VkImage image,
                              VkFormat format, VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView view;
  vkCreateImageView(kernel->logical_device.device, &viewInfo, nullptr, &view);
  return view;
}

void destroy_image_view(const ZeroG::Kernel *kernel, VkImageView view) {
  vkDestroyImageView(kernel->logical_device.device, view, nullptr);
}

VkDescriptorSetLayout
create_descriptor_set_layout(allocator *alloc, const Kernel *kernel,
                             const DescriptorSetCreateInfo *desc_info) {
  vk_dsl_array ds_array =
      create_vk_dsl_array(alloc, desc_info->descriptor_count);
  for (uint32_t i = 0; i < desc_info->descriptor_count; i++) {
    ds_array.data[i].binding = i;
    ds_array.data[i].descriptorCount = 1;
    ds_array.data[i].descriptorType =
        static_cast<VkDescriptorType>(desc_info->types[i]);
    ds_array.data[i].stageFlags =
        static_cast<VkShaderStageFlagBits>(desc_info->stages[i]);
    ds_array.data[i].pImmutableSamplers = nullptr;
  }
  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = desc_info->descriptor_count;
  layout_info.pBindings = ds_array.data;

  VkDescriptorSetLayout layout;
  vkCreateDescriptorSetLayout(kernel->logical_device.device, &layout_info,
                              nullptr, &layout);
  destroy_vk_dsl_array(alloc, ds_array);
  return layout;
}

void destroy_descriptor_set_layout(const Kernel *kernel,
                                   VkDescriptorSetLayout desc_layout) {
  vkDestroyDescriptorSetLayout(kernel->logical_device.device, desc_layout,
                               nullptr);
}

VkPipelineLayout create_pipeline_layout(const Kernel *kernel,
                                        VkDescriptorSetLayout desc_layuout) {
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;          // Optional
  pipeline_layout_info.pSetLayouts = &desc_layuout; // Optional
  pipeline_layout_info.pushConstantRangeCount = 0;  // Optional
  pipeline_layout_info.pPushConstantRanges = 0;     // Optional

  VkPipelineLayout layout;
  vkCreatePipelineLayout(kernel->logical_device.device, &pipeline_layout_info,
                         nullptr, &layout);
  return layout;
}

void destroy_pipeline_layout(const Kernel *kernel,
                             VkPipelineLayout pipe_layout) {
  vkDestroyPipelineLayout(kernel->logical_device.device, pipe_layout, nullptr);
}

VkShaderModule create_shader_module(const Kernel *kenel, uint32_t *code,
                                    size_t length) {
  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = length;
  info.pCode = code;
  VkShaderModule module;
  vkCreateShaderModule(kenel->logical_device.device, &info, nullptr, &module);
  return module;
}

void destroy_shader_module(const Kernel *kernel, VkShaderModule module) {
  vkDestroyShaderModule(kernel->logical_device.device, module, nullptr);
}
} // namespace util
} // namespace vk
} // namespace ZeroG
