struct SwapchainImage {
    VkImage*             pImage;
    VkImageLayout        layout         = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAccessFlags        access         = 0;
    VkPipelineStageFlags pipeline_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkExtent3D           extent;
    
    void translate_to_layout(const VkImageLayout&, const VkAccessFlags&, const VkPipelineStageFlags&);
};

class Swapchain;

class RenderPass {
    ui32 used_images  = 2;
    bool is_msaa_used = false;
    Image msaa_image, depth_image;
    std::vector<VkFramebuffer> frame_buffers;
    public:
    
    Swapchain* pSwapchain;
    VkRenderPass render_pass;
    
    void create();
    void recreate();
    void destroy();
    void create_frame_buffers();
    
    void begin(const VkCommandBuffer&, const ui32&);
    void end(const VkCommandBuffer&);
    
    constexpr operator VkRenderPass() { return render_pass; }
    
    private:
    void create_depth_image();
    void create_msaa_image();
};

class Swapchain {
    using ResizeCallback = void (*)();
    VkSurfaceKHR             surface;
    public:
    ResizeCallback           resize_callback;
    Queue*                   pQueue;
    RenderPass*              pRenderPass;
    ui32                     current_image;

    VkSwapchainKHR           swapchain;    
    std::vector<VkImage>     images;
    std::vector<VkImageView> views;

    VkExtent2D               extent;
    VkSurfaceFormatKHR       format;

    VkViewport               viewport;
    VkRect2D                 scissor;

    std::vector<SwapchainImage> meta_images;

    constexpr Swapchain(Queue* pQueue, RenderPass* pRenderPass) : pQueue(pQueue), pRenderPass(pRenderPass) { pRenderPass->pSwapchain = this; }
    constexpr operator VkSwapchainKHR() { return swapchain; }

    static void _destroy();

    void create();
    void recreate();
    void destroy();

    void get_images();
    void create_views();

    void acquire_image(const VkSemaphore&);
    void return_image(const VkSemaphore&);
    
    constexpr void set_resize_callback(ResizeCallback&& resize_callback) {
        this->resize_callback = resize_callback;
    }

    private:
    void _create();

    void get_surface();
    void get_format();
    constexpr void get_extent();
    
    VkSurfaceCapabilitiesKHR get_surface_capabilities();
    VkPresentModeKHR         get_present_mode();
};

template<typename... Vertices>
class GraphicPipeline : public Pipeline {
    RenderPass* pRenderPass;
    public:
    constexpr GraphicPipeline(RenderPass* pRenderPass) : pRenderPass(pRenderPass) {
        bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    }
    void create();
};

class Window {
    typedef int  Key;
    typedef bool IsActive;

    struct KeyInfo {
        Key key;
        int state;
    };
  
    GLFWwindow* window;
    bool        is_open = true;

    constexpr void check_keys();
    public:
    std::queue<KeyInfo>     keys_queue;
    std::map<Key, IsActive> keys_states;
    
    static void _init();
    void init();
    constexpr operator GLFWwindow*() { return window; }
    constexpr bool operator[](const int& key) { return keys_states[key]; } 
    operator bool() {
        glfwPollEvents(); 
        check_keys(); 
        is_open = !glfwWindowShouldClose(window);
        return is_open;
    }
    constexpr const bool& is_alive() const { return is_open; }
};