struct Instance {
    VkInstance instance;
    void create();
    void destroy();
    constexpr operator VkInstance() { return instance; }
};

struct Device {
    VkDevice device;
    VkPhysicalDevice physical_device;
    void create();
    void destroy();
    constexpr operator VkDevice() { return device; }
    constexpr operator VkPhysicalDevice() { return physical_device; }
    
    private:
    void select_physical_device();
    ui32 select_family_queue(VkQueueFlags);
};

struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkShaderModule shader_module;

    VkPipelineBindPoint bind_point;

    void destroy();
    void create_layout();
    void bind(const VkCommandBuffer&);

    inline static ResourceCollector<Pipeline> resource_collector = {};

    std::vector<VkDescriptorSetLayout> descriptor_layouts;
    std::vector<VkPushConstantRange>   push_constants;
};

struct Queue {
    struct QueueInfo {
        Queue*       queue;
        VkQueueFlags flags;
    };

    VkQueue       queue;
    VkCommandPool pool;
    ui32      queue_indice;

    constexpr Queue(VkQueueFlags flags);
    void destroy();

    void create_command_pool();

    constexpr operator VkQueue()       { return queue; }
    constexpr operator VkCommandPool() { return pool; }
    constexpr operator ui32()          { return queue_indice; }

    inline static std::vector<QueueInfo> queues_info = {};
    inline static ResourceCollector<Queue> resource_collector = {};
};