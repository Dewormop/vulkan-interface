struct ResourceInfo {
    enum class Type {
        BUFFER,
        BUFFER_VIEW,
        IMAGE
    } type;

    union {
        VkDescriptorBufferInfo buffer_info;
        VkDescriptorImageInfo  image_info;
        VkBufferView           buffer_view_info;
    } info;

    VkDescriptorType   resource_type;
    VkShaderStageFlags resource_shader_stage;
    ui64               resource_id;
};

class Descriptor {
    std::vector<VkDescriptorSet> sets;
    VkDescriptorPool             pool;
    Pipeline*                    pPipeline;
    public:
    VkDescriptorSetLayout        layout;
    std::vector<ResourceInfo*>   resources_info;

    inline static ResourceCollector<Descriptor> resource_collector = {};

    Descriptor(Pipeline* pPipeline, ui32 sets_count) : pPipeline(pPipeline), sets(sets_count) {}
    VkDescriptorSet& operator[] (ui32 i) { return sets[i]; }

    void create_layout();
    void create_pool();
    void allocate_sets();
    constexpr void add_resource(ResourceInfo*);
    void update_resource(const ui64&);
    void bind(const VkCommandBuffer&, ui32);
    void destroy();
};

template<typename Type>
struct PushConstant {
    VkPushConstantRange range {};
    Pipeline*           pPipeline;
    constexpr PushConstant(Pipeline* pPipeline, const VkShaderStageFlags& shader_stage) {
        range.size = sizeof(Type);
        range.stageFlags = shader_stage;

        this->pPipeline = pPipeline;
        pPipeline->push_constants.push_back(range);
    }
    void push(const VkCommandBuffer& command_buffer, const Type& data) {
        vkCmdPushConstants(command_buffer, pPipeline->layout, range.stageFlags, range.offset, range.size, &data);
    }
};

class CommandBuffers {
    Queue* pQueue;
    i32 setted_command_buffer = -1;
    public:
    std::vector<VkCommandBuffer> buffers;
    inline static ResourceCollector<CommandBuffers> resource_collector = {};

    constexpr CommandBuffers(Queue* pQueue) : pQueue(pQueue) {}
    constexpr VkCommandBuffer& operator[](const ui32& i) { return buffers[i]; }
    constexpr operator VkCommandBuffer() { return buffers[setted_command_buffer]; }

    void allocate();
    void allocate(const ui64&);
    void submit(const ui32&);
    void begin(const ui32&);
    void end(const ui32& i) { _check(vkEndCommandBuffer, buffers[i]); }
    void destroy();
    
    constexpr void early_destroy();
    constexpr void set_buffer(const ui32& i) { setted_command_buffer = i; }

    void submit();
    void submit(const VkSemaphore&, const VkPipelineStageFlags&, const VkSemaphore&, const VkFence&);
    void begin();
    void end() { _check(vkEndCommandBuffer, buffers[setted_command_buffer]); }
};

struct SwapchainImage;

class Image {
    VkDeviceMemory     memory;
    VkImageAspectFlags aspect;
    Queue*             pQueue;
    Descriptor*        pDescriptor;

    constexpr VkImageLayout& get_layout() { return descriptor_resource.info.image_info.imageLayout; }
    VkImage            image;

    public:
    ResourceInfo       descriptor_resource {.type = ResourceInfo::Type::IMAGE};
    VkFormat           format;

    VkPipelineStageFlags pipeline_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags        access         = 0;
    
    VkExtent3D           extent;

    inline static ResourceCollector<Image> resource_collector = {};
    
    constexpr Image() { this->descriptor_resource.info.image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED; }

    constexpr VkImageView& get_view()  { return descriptor_resource.info.image_info.imageView; }
    constexpr VkImage&     get_image() { return image; }
    constexpr operator VkImage() { return image; }

    void create(const ui32&, const ui32&, Queue*, const VkSampleCountFlagBits&, const VkFormat&, const VkImageUsageFlags&, const VkMemoryPropertyFlags&, bool);
    void create_view(VkImageAspectFlags);
    void destroy();
    void translate_to_layout(const VkImageLayout&, const VkAccessFlags&, const VkPipelineStageFlags&);
    void transfer(Image&);
    void transfer(SwapchainImage&);
    void copy(Image&);
    void copy(SwapchainImage&);
    constexpr void set_descriptor(Descriptor*, const VkDescriptorType&, const VkShaderStageFlags&);
};

struct BufferBase {
    inline static ResourceCollector<BufferBase> resource_collector = {};
    ui64 resource_id;
    VkDeviceMemory memory;
    ResourceInfo   descriptor_resource = {.type = ResourceInfo::Type::BUFFER};
    void destroy();
    void early_destroy();
};

template<typename Type>
class Buffer : public BufferBase {
    bool        is_mapped = false;
    Descriptor* pDescriptor;
    public:
    Type*       memory_data = nullptr;

    constexpr Type& operator[] (const ui32& i) { return memory_data[i]; }
    constexpr Type& back() { return memory_data[get_size() / sizeof(Type) - 1]; }
    constexpr const ui64& get_size() { return descriptor_resource.info.buffer_info.range; }
    constexpr ui32  get_count() { return get_size() / sizeof(Type); }
    constexpr operator VkBuffer() const { return descriptor_resource.info.buffer_info.buffer; }
    constexpr VkBuffer* operator& () { return &descriptor_resource.info.buffer_info.buffer; }

    void create(const ui64&, const VkBufferUsageFlags&, const VkMemoryPropertyFlags&);
    void create_stage_buffer(const ui64&);
    void create_stage_buffer(const ui64&, const Type*);
    void create_stage_buffer(Buffer&);
    void map_memory();
    void unmap_memory();
    void gpu_copy(const ui64&, const Type*);
    void gpu_copy(Buffer&);
    void gpu_insert(Buffer&, const std::vector<ui32>&);
    constexpr void memory_copy(const ui64&, const Type*);
    constexpr void set_descriptor(Descriptor*, const VkDescriptorType&, const VkShaderStageFlags&);
};

class Fences {  
    std::vector<VkFence> fences;
    public:
    inline static ResourceCollector<Fences> resource_collector = {};

    void create(const ui64&);
    void destroy();
    bool wait(const ui64& i);
    constexpr VkFence& operator[](const ui64& i) { return fences[i]; }
};

class Semaphores {
    std::vector<VkSemaphore> semaphores;
    public:
    inline static ResourceCollector<Semaphores> resource_collector = {};

    void create(const ui64&);
    void destroy();
    constexpr VkSemaphore& operator[](const ui64& i) { return semaphores[i]; }
};

struct ComputePipeline : public Pipeline {
    constexpr ComputePipeline() { bind_point = VK_PIPELINE_BIND_POINT_COMPUTE; }
    void create();
};