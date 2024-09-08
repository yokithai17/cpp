#include <memory>

struct BaseControlBlock {
    size_t shared_count = 0;
    size_t weak_count = 0;

    virtual void* get() = 0;
    virtual void destroy_obj() = 0;
    virtual void destruct_block() = 0;
};

template <typename T>
class SharedPtr {
private:
    template <typename Y, typename Alloc, typename... Args>
    friend SharedPtr<Y> allocateShared(Alloc, Args&&...);

    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;

    BaseControlBlock* control_block_;
    T* ptr_;

    explicit SharedPtr(BaseControlBlock* block) : control_block_(block) {
        if (!block) {
            control_block_ = nullptr;
            return;
        }
        ptr_ = reinterpret_cast<T*>(block->get());
        ++control_block_->shared_count;
    }

public:
    SharedPtr() : control_block_(nullptr), ptr_(nullptr) {}

    template <typename Y, typename Deleter = std::default_delete<Y>,
            typename Alloc = std::allocator<Y>>
    SharedPtr(Y* ptr, Deleter deleter = Deleter(), Alloc alloc = Alloc()) : ptr_(ptr) {
        using control_block_type = ControlBlockPointer<Y, Deleter, Alloc>;
        typename std::allocator_traits<Alloc>::template rebind_alloc<control_block_type> block_alloc =
                alloc;
        auto control_block = block_alloc.allocate(1);
        new (control_block) control_block_type(ptr, deleter, alloc);
        control_block_ = control_block;
        ++control_block_->shared_count;
    }

    SharedPtr(const SharedPtr& other) : SharedPtr(other.control_block_) {}

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        ++control_block_->shared_count;
    }

    SharedPtr(SharedPtr&& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        other.control_block_ = nullptr;
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        other.control_block_ = nullptr;
    }

    ~SharedPtr() {
        if (!control_block_) return;
        --control_block_->shared_count;
        if (!use_count()) {
            control_block_->destroy_obj();
            if (control_block_->weak_count == 0) {
                control_block_->destruct_block();
                return;
            }
        }
    }

    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr ptr = other;
        swap(ptr);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr ptr = std::move(other);
        swap(ptr);
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(Y&& other) {
        SharedPtr ptr = std::forward<Y>(other);
        swap(ptr);
        return *this;
    }

    template <typename Y, typename Deleter, typename Alloc>
    struct ControlBlockPointer : public BaseControlBlock {
        Deleter deleter_;
        Alloc alloc_;
        Y* ptr_;

        ControlBlockPointer(Y* ptr, Deleter deleter, Alloc alloc)
                : deleter_(deleter), alloc_(alloc), ptr_(ptr) {}

        void* get() override { return ptr_; }

        void destroy_obj() override { deleter_(ptr_); }

        void destruct_block() override {
            using alloc_traits = std::allocator_traits<Alloc>;
            typename alloc_traits::template rebind_alloc<ControlBlockPointer> block_alloc = alloc_;
            block_alloc.deallocate(this, 1);
        }
    };

    template <typename Alloc>
    struct MakeSharedControlBlock : public BaseControlBlock {
        using alloc_block = typename std::allocator_traits<Alloc>::template rebind_alloc<MakeSharedControlBlock>;
        using alloc_traits_block = std::allocator_traits<alloc_block>;
        using alloc_object = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
        using alloc_traits_object = std::allocator_traits<alloc_object>;

        std::aligned_storage<sizeof(T), alignof(T)> space_;
        Alloc alloc_;

        template <typename... Args>
        explicit MakeSharedControlBlock(Alloc alloc, Args&&... args) : alloc_(std::move(alloc)) {
            alloc_object object_allocator(alloc_);
            alloc_traits_object::construct(object_allocator, reinterpret_cast<T*>(&space_),
                                           std::forward<Args>(args)...);
        }

        void* get() override { return reinterpret_cast<T*>(&space_); }

        void destroy_obj() override {
            alloc_object object_allocator(alloc_);
            alloc_traits_object::destroy(object_allocator, reinterpret_cast<T*>(&space_));
        }

        virtual ~MakeSharedControlBlock() = default;

        void destruct_block() override {
            auto this_pointer = this;
            alloc_block block_allocator(std::move(alloc_));
            this_pointer->~MakeSharedControlBlock();
            alloc_traits_block::deallocate(block_allocator, this_pointer, 1);
        }
    };

    void swap(SharedPtr& other) {
        std::swap(control_block_, other.control_block_);
        std::swap(ptr_, other.ptr_);
    }

    size_t use_count() const { return control_block_->shared_count; }

    void reset() {
        if (control_block_) {
            --control_block_->shared_count;
            if (use_count() == 0) {
                control_block_->destroy_obj();
                if (control_block_->weak_count == 0) {
                    control_block_->destruct_block();
                }
            }
            control_block_ = nullptr;
            ptr_ = nullptr;
        }
    }

    template <typename Y>
    void reset(Y* ptr) {
        reset();
        *this = SharedPtr(ptr);
    }

    T* get() const { return ptr_; }

    T* operator->() const { return ptr_; }

    T& operator*() const { return *operator->(); }
};

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(Alloc alloc, Args&&... args) {
    using control_block_type = typename SharedPtr<T>::template MakeSharedControlBlock<Alloc>;
    typename std::allocator_traits<Alloc>::template rebind_alloc<control_block_type> block_alloc = alloc;
    auto control_block = block_alloc.allocate(1);
    new (control_block) control_block_type(alloc, std::forward<Args>(args)...);
    auto block = static_cast<BaseControlBlock*>(control_block);
    return SharedPtr<T>(block);
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return allocateShared<T, std::allocator<T>>(std::allocator<T>(), std::forward<Args>(args)...);
}

template <typename T>
class WeakPtr {
private:
    template <typename Y>
    friend class SharedPtr;

    BaseControlBlock* control_block_;

public:
    WeakPtr() : control_block_(nullptr) {}

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& shared_ptr) : control_block_(shared_ptr.control_block_) {
        ++control_block_->weak_count;
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) : control_block_(other.get_cb()) {
        ++control_block_->weak_count;
    }

    WeakPtr(const WeakPtr& other) : control_block_(other.control_block_) {
        ++control_block_->weak_count;
    }

    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        WeakPtr<Y>(other).swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        WeakPtr<Y>(std::move(other)).swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        WeakPtr<Y>(other).swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(SharedPtr<Y>&& other) {
        WeakPtr<Y>(std::move(other)).swap(*this);
        return *this;
    }

    void swap(WeakPtr& other) { std::swap(control_block_, other.control_block_); }

    size_t use_count() const { return control_block_->shared_count; }

    bool expired() const { return use_count() == 0; }

    SharedPtr<T> lock() const {
        if (expired()) return SharedPtr<T>();
        return SharedPtr<T>(control_block_);
    }

    BaseControlBlock* get_cb() const { return control_block_; }

    ~WeakPtr() {
        if (!control_block_) return;

        --control_block_->weak_count;
        if (control_block_->shared_count == 0 && control_block_->weak_count == 0) {
            control_block_->destruct_block();
        }
    }
};

template <typename T>
class EnableSharedFromThis {
private:
    template <typename Y, typename Alloc, typename... Args>
    friend SharedPtr<Y> allocateShared(Alloc, Args&&...);

    WeakPtr<T> ptr_;

public:
    SharedPtr<T> shared_from_this() const { return ptr_.lock(); }
};