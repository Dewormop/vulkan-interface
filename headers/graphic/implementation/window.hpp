static void keys_callback(GLFWwindow* window, i32 key, i32, i32 action, i32) {
    ::window.keys_queue.push({key, action});   
}
static void window_resize(GLFWwindow* window, int width, int height) {
    WINDOW_SIZE = {(ui32) width, (ui32) height};
    _used_swapchain->pRenderPass->recreate();

    _used_swapchain->resize_callback();
}

void Window::init() {
    if (glfwInit() == GLFW_FALSE) throw std::runtime_error("failed to initiate glfw");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_SIZE[0], WINDOW_SIZE[1], "", nullptr, nullptr);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    glfwSetKeyCallback(window, keys_callback);
    glfwSetWindowSizeCallback(window, window_resize);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorWorkarea(monitor, nullptr, nullptr, (i32*) &WINDOW_MAX_SIZE[0], (i32*) &WINDOW_MAX_SIZE[1]);

    glfwSetWindowPos(window, (WINDOW_MAX_SIZE[0] - WINDOW_SIZE[0]) / 2, (WINDOW_MAX_SIZE[1] - WINDOW_SIZE[1]) / 2);

    glfwShowWindow(window);
}

constexpr void Window::check_keys() {
    while (keys_queue.size() > 0) {
        auto& key_info            = keys_queue.front();
        keys_states[key_info.key] = key_info.state == GLFW_PRESS || key_info.state == GLFW_REPEAT;
        keys_queue.pop();
    }
}

void Window::_init() {
    ::window.init();
}
