void Swapchain::get_surface() {
    _check(glfwCreateWindowSurface, instance, window, nullptr, &surface);
}

constexpr void Swapchain::get_extent() {
    WINDOW_CENTER_POSITION = {WINDOW_SIZE[0] / 2, WINDOW_SIZE[1] / 2};
    
    extent = {WINDOW_SIZE[0], WINDOW_SIZE[1]};
    viewport.width  = extent.width;
    viewport.height = extent.height;
    scissor.extent  = extent;
}

void Swapchain::get_format() {
    get_elements(VkSurfaceFormatKHR, vkGetPhysicalDeviceSurfaceFormatsKHR, device, surface);

    for (auto& format : elements) if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        this->format = format;
        return;
    }

    throw std::runtime_error(collect("failed to find surface format: [", VK_FORMAT_B8G8R8A8_SRGB,"], color space: [", VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,"]"));
}

VkPresentModeKHR Swapchain::get_present_mode() {
    get_elements(VkPresentModeKHR, vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface);

    for (auto& mode : elements) {
        if (mode == VK_PRESENT_MODE_FIFO_KHR) return mode;
    }

    return VK_PRESENT_MODE_MAILBOX_KHR;
}

VkSurfaceCapabilitiesKHR  Swapchain::get_surface_capabilities() {
    VkSurfaceCapabilitiesKHR capabilities;
    _check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, device, surface, &capabilities);
    return capabilities;
}

void Swapchain::create() {
    _used_swapchain = this;

    get_surface();
    get_format();
    get_present_mode();

    viewport = {0, 0, 0, 0, 0, 1};
    scissor  = {{}, {}};

    _create();
}

void Swapchain::recreate() {
    auto _swapchain = swapchain;
    for (auto& view : views) vkDestroyImageView(device, view, nullptr);

    _create();

    vkDestroySwapchainKHR(device, _swapchain, nullptr);
}

void Swapchain::_create() {
    get_extent();
    get_surface_capabilities();
    VkPresentModeKHR         mode                 = get_present_mode();
    VkSurfaceCapabilitiesKHR surface_capabilities = get_surface_capabilities();;

    VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    info.clipped = true;
    info.imageArrayLayers      = 1;
    info.minImageCount         = 3;
    info.queueFamilyIndexCount = 1;
    info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.imageColorSpace       = format.colorSpace;
    info.imageFormat           = format.format;
    info.imageExtent           = extent;
    info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.presentMode           = mode;
    info.preTransform          = surface_capabilities.currentTransform;
    info.pQueueFamilyIndices   = &pQueue->queue_indice;
    info.surface               = surface;
    info.oldSwapchain          = swapchain;

    _check(vkCreateSwapchainKHR, device, &info, nullptr, &swapchain);

    get_images();
    create_views();
}

void Swapchain::destroy() {
    vkQueueWaitIdle(*pQueue);

    vkDestroySwapchainKHR(device, swapchain, nullptr);
    for (auto& view : views) vkDestroyImageView(device, view, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

    pRenderPass->destroy();
}

void Swapchain::get_images() {
    set_elements(images, vkGetSwapchainImagesKHR, device, swapchain);
    views.resize(images.size());
    meta_images.resize(images.size());
    for (int i = 0; i < meta_images.size(); ++i) {
        meta_images[i].pImage = &images[i];
        meta_images[i].extent = {extent.width, extent.height, 1};
    }
}

void Swapchain::create_views() {
    VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.format = format.format;
    info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    info.viewType         = VK_IMAGE_VIEW_TYPE_2D;

    for (i32 i = 0; i < views.size(); ++i) {
        info.image = images[i];
        check(vkCreateImageView(device, &info, nullptr, &views[i]), "swapchain view image create:", i);
    }
}

void Swapchain::acquire_image(const VkSemaphore& acquire_semaphore) {
    _check(vkAcquireNextImageKHR, device, swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &current_image);
}

void Swapchain::return_image(const VkSemaphore& present_semaphore) {
    static VkPresentInfoKHR present_info {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .waitSemaphoreCount = 1, .swapchainCount = 1};
    
    present_info.pSwapchains        = &swapchain;
    present_info.pWaitSemaphores    = &present_semaphore;
    present_info.pImageIndices      = &current_image;

    _check(vkQueuePresentKHR, *pQueue, &present_info);
}

void SwapchainImage::translate_to_layout(const VkImageLayout& new_layout, const VkAccessFlags& new_access, const VkPipelineStageFlags& new_stage) {
    static VkImageMemoryBarrier barrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate(1);
    command_buffer.set_buffer(0);
    command_buffer.begin();

    barrier.image            = *pImage;
    barrier.oldLayout        = layout;
    barrier.srcAccessMask    = access;
    barrier.newLayout        = new_layout;
    barrier.dstAccessMask    = new_access;

    vkCmdPipelineBarrier(command_buffer, pipeline_stage, new_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    command_buffer.end();
    command_buffer.submit();
    command_buffer.resource_collector.remove_back();

    layout         = new_layout;
    access         = new_access;
    pipeline_stage = new_stage;
}

void Image::transfer(SwapchainImage& dst_image) {
    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate(1);
    command_buffer.set_buffer(0);
    command_buffer.begin();

    static VkImageCopy copy_region {.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}};
    copy_region.srcSubresource = {this->aspect, 0, 0, 1};
    copy_region.extent         = extent;
    vkCmdCopyImage(command_buffer, *this, this->get_layout(), *dst_image.pImage, dst_image.layout, 1, &copy_region);

    command_buffer.end();
    command_buffer.submit();
    command_buffer.resource_collector.remove_back();
}

void Image::copy(SwapchainImage& src_image) {
    CommandBuffers command_buffer(&transfer_queue);
    command_buffer.allocate(1);
    command_buffer.set_buffer(0);
    command_buffer.begin();

    static VkImageCopy copy_region {.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}};
    copy_region.dstSubresource = {aspect, 0, 0, 1};
    copy_region.extent         = src_image.extent;
    vkCmdCopyImage(command_buffer, *src_image.pImage, src_image.layout, *this, this->get_layout(), 1, &copy_region);
    
    command_buffer.end();
    command_buffer.submit();
    command_buffer.resource_collector.remove_back();
}

void Swapchain::_destroy() {
    _used_swapchain->destroy();
}