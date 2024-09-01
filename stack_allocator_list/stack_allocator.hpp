#include <memory>

template <std::size_t N>
class stack_storage {
public:
    stack_storage() : buff(), curr(buff) {}
    
    stack_storage(const stack_storage&) = delete;

    template <typename T, size_t M>
    friend class stack_allocator;

private:
    char* reserve(std::size_t amount) {
        if ((curr - buff) + amount >= N) {
            return nullptr;
        }
        cur += amount;
        return cur - amount;
    }

    void allign(std::size_t aligment) {
        std::size_t space = N - (curr - buff);
        curr = std::align(aligment, aligment
                        , static_cast<void*>(curr), space);
    }

    char buff[N];
    char* curr;
};

template <typename T, std::size_t N>
class stack_allocator {
public:
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    explicit stack_allocator() : buff(&stack_storage<N>()) {}

    explicit stack_allocator(stack_storage<N>& st) : buff(&st) {}
    
    template <typename U>
    explicit stack_allocator(const stack_allocator<U, N>& other)
        : buff(const_cast<stack_storage<N>*>(other.buff)) {}

    template <typename U>
    stack_allocator<T>& operator=(const stack_allocator<U, N>& other) {
        buff = other.buff;
        return *this;
    }

    T* allocate(std::size_t count) {
        buff->allign(alignof(T));
        return reinterpret_cast<T*>(buff->reserve(count));
    }

    void deallocate(T* ptr, std::size_t count) {}

    template <typename U>
    struct rebind {
        using other = stack_allocator<U, N>;
    };
private:
    stack_storage<N>* buff;
};