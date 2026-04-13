template<typename Arg>
constexpr std::string _collect(const Arg& arg) {
    std::stringstream out;
    out << arg;
    return out.str();
}

template<>
constexpr std::string _collect(const glm::vec4& args) {
    std::string out = "[ ";
    for (int i = 0; i < args.length(); ++i) out += _collect(args[i]) + " ";
    out += "]";
    return out;
}

template<>
constexpr std::string _collect(const glm::vec3& args) {
    std::string out = "[ ";
    for (int i = 0; i < args.length(); ++i) out += _collect(args[i]) + " ";
    out += "]";
    return out;
}

template<>
constexpr std::string _collect(const glm::mat4& args) {
    std::string out = "[\n";
    for (int i = 0; i < args.length(); ++i) out += "    " + _collect(args[i]) + "\n";
    out += "]";
    return out;
}

template<typename Arg, typename... Args>
constexpr std::string collect(const Arg& arg, const Args&... args) {
    if constexpr (sizeof...(Args) > 0) return _collect(arg) + " " + collect(args...);
    return _collect(arg);
}

template<typename Arg, typename... Args>
constexpr void print(const Arg& arg, const Args&... args) {
    std::cout << collect(arg, args...) << "\n";
}

void resize_window();
template<typename Arg, typename... Args>
constexpr void check(const VkResult& result, const Arg& arg, const Args&... args) {
    // if (result == VK_SUCCESS) print(arg, args..., "| result:", result);
    if (result != VK_SUCCESS) throw std::runtime_error(collect(arg, args..., "| result:", result));
}
#define _check(function, function_arguments...) check(function(function_arguments), #function);

#define get_elements(element_type, elements_function, function_arguments...) \
    uint32_t elements_count;\
    _check(elements_function, function_arguments, &elements_count, nullptr);\
    std::vector<element_type> elements(elements_count); \
    _check(elements_function, function_arguments, &elements_count, elements.data());

#define get_elements_nocheck(element_type, elements_function, function_arguments...) \
    uint32_t elements_count;\
    elements_function(function_arguments, &elements_count, nullptr);\
    std::vector<element_type> elements(elements_count);\
    elements_function(function_arguments, &elements_count, elements.data());

#define set_elements(elements, elements_function, function_arguments...) \
    uint32_t elements_count;\
    _check(elements_function, function_arguments, &elements_count, nullptr);\
    elements.resize(elements_count); \
    _check(elements_function, function_arguments, &elements_count, elements.data());

template<typename Arg, typename... Args>
constexpr void log_error(const Arg& arg, const Args&... args) {
    throw std::runtime_error(collect(arg, args...));
}

template<typename Arg, typename... Args>
constexpr void log_warning(const Arg& arg, const Args&... args) {
    print("WARNING:", arg, args...);
}

template<typename Arg, typename... Args>
constexpr void log_info(const Arg& arg, const Args&... args) {
    print("INFO:", arg, args...);
}

template<typename T>
constexpr auto fast_mod(const T& value, const T& ceil) {
    return value >= ceil ? value % ceil : value;
}