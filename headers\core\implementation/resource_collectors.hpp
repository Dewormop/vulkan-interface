template<typename _Resource, typename... _Resources>
constexpr MainResourceCollector<_Resource, _Resources...>::MainResourceCollector() {
    sub_resource_collectors = (void**) malloc(sizeof(void*) * (1 + sizeof...(_Resources)));

    create_elements<_Resource, _Resources...>(0);
}

template<typename _Resource, typename... _Resources>
constexpr void MainResourceCollector<_Resource, _Resources...>::destroy() {
    destroy_elements<_Resource, _Resources...>(0);

    free(sub_resource_collectors);
}