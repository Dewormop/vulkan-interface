void RenderPass::create_depth_image() {
    depth_image.create(
        WINDOW_SIZE[0], WINDOW_SIZE[1], pSwapchain->pQueue, SAMPLE_COUNT, DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, DEVICE_PROPERTY, true
    );
    depth_image.create_view(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    depth_image.translate_to_layout(
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
    );
}

void RenderPass::create_msaa_image() {
    msaa_image.create(
        WINDOW_SIZE[0], WINDOW_SIZE[1], pSwapchain->pQueue, SAMPLE_COUNT, pSwapchain->format.format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, DEVICE_PROPERTY, true
    );
    msaa_image.create_view(VK_IMAGE_ASPECT_COLOR_BIT);
}

void RenderPass::create_frame_buffers() {
    VkFramebufferCreateInfo info {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    info.renderPass = render_pass;
    info.attachmentCount = used_images;
    info.width = WINDOW_SIZE[0];
    info.height = WINDOW_SIZE[1];
    info.layers = 1;
    for (i32 i = 0; i < frame_buffers.size(); ++i) {
        std::vector<VkImageView> views { pSwapchain->views[i], depth_image.get_view(), msaa_image.get_view() };
        info.pAttachments = views.data();
        _check(vkCreateFramebuffer, device, &info, nullptr, &frame_buffers[i]);
    }
}

void RenderPass::create() {
    if (SAMPLE_COUNT != VK_SAMPLE_COUNT_1_BIT) {
        used_images = 3;
        is_msaa_used = true;
    }

    VkAttachmentDescription swapchain_attachment {};
    swapchain_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swapchain_attachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    swapchain_attachment.format        = pSwapchain->format.format;
    swapchain_attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    swapchain_attachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    swapchain_attachment.samples       = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentDescription depth_attachment {};
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.format        = DEPTH_FORMAT;
    depth_attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.samples       = SAMPLE_COUNT;

    VkAttachmentDescription msaa_attachment = swapchain_attachment;
    msaa_attachment.samples = SAMPLE_COUNT;

    VkAttachmentDescription attachments[] = {
        swapchain_attachment,
        depth_attachment,
        msaa_attachment
    };

    VkAttachmentReference swapchain_reference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_reference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkAttachmentReference msaa_reference{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDependency subpass_dependency {};
    subpass_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass    = 0;
    subpass_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass_description {};
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.pColorAttachments = &swapchain_reference;
    subpass_description.pDepthStencilAttachment = &depth_reference;

    if (is_msaa_used) {
        subpass_description.pColorAttachments = &msaa_reference;
        subpass_description.pResolveAttachments = &swapchain_reference;
    }

    VkRenderPassCreateInfo info {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = used_images;
    info.dependencyCount = 1;
    info.subpassCount    = 1;
    info.pAttachments    = attachments;
    info.pDependencies   = &subpass_dependency;
    info.pSubpasses      = &subpass_description;

    _check(vkCreateRenderPass, device, &info, nullptr, &render_pass);

    frame_buffers.resize(pSwapchain->images.size());
    
    create_depth_image();
    if (is_msaa_used) create_msaa_image();
    create_frame_buffers();
}

void RenderPass::recreate() {
    vkQueueWaitIdle(*pSwapchain->pQueue);

    for (auto& frame_buffer : frame_buffers) vkDestroyFramebuffer(device, frame_buffer, nullptr);
    pSwapchain->recreate();

    depth_image.destroy();
    create_depth_image();
    
    if (is_msaa_used) {
        msaa_image.destroy();
        create_msaa_image();
    }

    create_frame_buffers();
}

void RenderPass::destroy() {
    vkQueueWaitIdle(*pSwapchain->pQueue);

    for (auto& frame_buffer : frame_buffers) vkDestroyFramebuffer(device, frame_buffer, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);
    if (is_msaa_used) msaa_image.destroy();
    depth_image.destroy();
}

void RenderPass::begin(const VkCommandBuffer& command_buffer, const ui32& image_indice) {
    constexpr static std::array<VkClearValue, 3> clear_values{{
        {.color = {0.6, 0.6, 0.6, 1}},
        {.depthStencil = {1, 0}},
        {.color = {0.6, 0.6, 0.6, 1}}
    }};
    static VkRenderPassBeginInfo info {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .clearValueCount = used_images,
        .pClearValues = clear_values.data()
    };

    info.framebuffer     = frame_buffers[image_indice];
    info.renderPass      = render_pass;
    info.renderArea      = {.extent = pSwapchain->extent};

    vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(const VkCommandBuffer& command_buffer) {
    vkCmdEndRenderPass(command_buffer);
}