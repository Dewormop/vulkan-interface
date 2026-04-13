void Instance::create() {
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_4;

    ui32 count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char*> layers {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.pApplicationInfo        = &app_info;
    info.enabledExtensionCount   = count;
    info.enabledLayerCount       = (ui32) layers.size();
    info.ppEnabledExtensionNames = extensions;
    info.ppEnabledLayerNames     = layers.data();

    _check(vkCreateInstance, &info, nullptr, &instance);
}

void Instance::destroy() {
    vkDestroyInstance(instance, nullptr);
}