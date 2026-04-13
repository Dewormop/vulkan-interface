typedef uint8_t ui8;
typedef int8_t  i8;
typedef uint16_t ui16;
typedef int16_t  i16;
typedef uint32_t ui32;
typedef int32_t  i32;
typedef uint64_t ui64;
typedef int64_t  i64;

typedef float f32;
typedef double f64;

template<typename Resource> 
struct ResourcesConteiner {
    std::vector<Resource*> pResources;
    constexpr void append(Resource* pResource) { pResources.push_back(pResource); }
    constexpr Resource* operator[] (const ui32& i) const { return pResources[i]; }
    constexpr ui64 get_size() const { return pResources.size(); }
    constexpr void remove(const ui32& i) { pResources.erase(pResources.begin() + i); }
    constexpr void remove_back() { remove(get_size() - 1); }
    constexpr void start_action(void (*&&func)(Resource*)) { for (auto& pResource : pResources) func(pResource); }
    // On failure returns -1
    constexpr i32 find_resource(Resource* pSearchResource) {
        for (i32 i = 0; auto& pResource : pResources) if (i++; pResource == pSearchResource) return i - 1;
        return -1;
    }
};

template<typename _Resource>
struct ResourceCollector : public ResourcesConteiner<_Resource> {
    constexpr void remove(ui32 i) {
        this->pResources[i]->destroy();
        ResourcesConteiner<_Resource>::remove(i);
    }
    constexpr void remove_back() {
        remove(this->get_size() - 1);
    }
    constexpr void destroy_all() {
        ui64 size = this->get_size();
        for (ui64 i = 0; i < size; ++i) remove_back();
    }
};

template<typename _Resource, typename... _Resources>
class MainResourceCollector {
    void** sub_resource_collectors;
    size_t size;

    template<typename __Resource, typename... __Resources>
    constexpr void create_elements(uint32_t index) {
        ((ResourceCollector<__Resource>**) sub_resource_collectors)[index] = &__Resource::resource_collector;

        if constexpr (sizeof...(__Resources) > 0) create_elements<__Resources...>(index + 1);
    }

    template<typename __Resource, typename... __Resources>
    constexpr void destroy_elements(uint32_t index) {
        ((ResourceCollector<__Resource>*) sub_resource_collectors[index])->destroy_all();

        if constexpr (sizeof...(__Resources) > 0) destroy_elements<__Resources...>(index + 1);
    }

    public:
    constexpr MainResourceCollector();
    constexpr void destroy();
};

#include "implementation/resources/resource_collectors.hpp"

#include "implementation/base/structures.hpp"
#include "implementation/resources/structures.hpp"