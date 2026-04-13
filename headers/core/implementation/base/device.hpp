void Device::select_physical_device() {
    get_elements(VkPhysicalDevice, vkEnumeratePhysicalDevices, instance);
    std::vector<VkPhysicalDevice> optional_devices;

    for (auto& device : elements) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_device = device;
            return;
        } else optional_devices.push_back(device);
    }
    physical_device = optional_devices.back();
}

uint32_t Device::select_family_queue(VkQueueFlags filter) {
    get_elements_nocheck(VkQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties, physical_device);

    for (int i = 0; i < elements_count; ++i) if (elements[i].queueFlags & filter) return i;
    throw std::runtime_error(collect("failed to find family queue by filter:", filter));
}

void Device::create() {
    struct size_offset { uint32_t count; uint32_t offset; };

    select_physical_device();
    
    float priority = 0.4;
    std::vector<VkDeviceQueueCreateInfo> queues_info;

    std::map<uint32_t, size_offset> queue_families_data;

    for (auto& queue_info : Queue::queues_info) {
        queue_info.queue->queue_indice = select_family_queue(queue_info.flags);
        queue_families_data[queue_info.queue->queue_indice].count += 1;
    }

    for (auto& [indice, data] : queue_families_data) {
        queues_info.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO});

        float* priorities = (float*) malloc(sizeof(float) * data.count);
        for (int i = 0; i < data.count; ++i) priorities[i] = priority;

        queues_info.back().pQueuePriorities = priorities;
        queues_info.back().queueCount       = data.count;
        queues_info.back().queueFamilyIndex = indice;
    }

    std::vector<const char*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_EXT_extended_dynamic_state3"
    };

    VkPhysicalDeviceRobustness2FeaturesEXT robustness {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};
    robustness.nullDescriptor = true;

    VkPhysicalDeviceVulkan11Features v11_features {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    v11_features.shaderDrawParameters = true;
    v11_features.pNext = &robustness;

    VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &v11_features};
    info.enabledExtensionCount   = (uint32_t) extensions.size();
    info.ppEnabledExtensionNames = extensions.data();
    info.queueCreateInfoCount    = (uint32_t) queues_info.size();
    info.pQueueCreateInfos       = queues_info.data();

    _check(vkCreateDevice, physical_device, &info, nullptr, &device);

    for (auto& queue_info : Queue::queues_info) {
        vkGetDeviceQueue(device, queue_info.queue->queue_indice, queue_families_data[queue_info.queue->queue_indice].offset++, &queue_info.queue->queue);
    }

    for (auto& queue_info : queues_info) free((void*) queue_info.pQueuePriorities);
}

void Device::destroy() {
    Queue::resource_collector.start_action([] (Queue* queue) {
        vkQueueWaitIdle(*queue);
    });
    vkDestroyDevice(device, nullptr);
}

constexpr Queue::Queue(VkQueueFlags flags) {
    queues_info.push_back({this, flags}); 
    resource_collector.append(this);
}
void Queue::destroy() { vkDestroyCommandPool(device, pool, nullptr); }
