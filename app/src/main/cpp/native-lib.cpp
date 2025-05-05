#include <algorithm>
#include <android/dlext.h>
#include <android/log.h>
#include <dlfcn.h>
#include <jni.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/resource.h>
#include <unistd.h>
#include <utility>
#include <vulkan/vulkan.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#define LOG_TAG "RPCSX"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

VkInstance instance;
VkSurfaceKHR surface;
VkPhysicalDevice physicalDevice;
VkDevice device;
VkQueue graphicsQueue;

ANativeWindow* window = nullptr;

bool createVulkanInstance() {
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "RPCSX",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "RPCSX Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1,
    };

    const char* extensions[] = { "VK_KHR_surface", "VK_KHR_android_surface" };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extensions,
    };

    return vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS;
}

bool createAndroidSurface() {
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = window
    };

    return vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) == VK_SUCCESS;
}

bool pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) return false;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& dev : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);
        physicalDevice = dev;
        return true;
    }

    return false;
}

bool createLogicalDevice() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int graphicsFamily = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
            break;
        }
    }

    if (graphicsFamily == -1) return false;

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = static_cast<uint32_t>(graphicsFamily),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
    };

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    return true;
}
