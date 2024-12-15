#pragma once

#include <cstddef>
#include <memory>
#include <utility>

template <typename T>
class shared_ptr;

template <typename T>
class weak_ptr;

/** @namespace detail contains MAIN IMPLEMENTATIONS of shared_ptr and weak_ptr
 *
 * base_control_block contains {shared_count, weak_count}
 *
 *  template <U, Deleter, Alloc>
 *  control_block_regular : base_control_block
 *
 * template <U, Alloc>
 * control_block_make_shared : base_control_block
 *
 * @tparam U value @tparam Deleter deleter Value?)
 * @tparam Alloc allocator of block and value ?????
 * @param U* ptr_ @param Deleter del_ @param Alloc alloc_
 * @param size_t shared_Count @param size_t weak_count
 */
namespace detail {
struct base_control_block {
  virtual ~base_control_block() = default;

  virtual void destroy_data() noexcept = 0;

  virtual void destroy_block() noexcept = 0;

  void incr_shared() noexcept {
    ++shared_count;
  }

  void incr_weak() noexcept {
    ++weak_count;
  }

  void decr_shared() noexcept {
    if (--shared_count == 0) {
      destroy_data();
      decr_weak();
    }
  }

  void decr_weak() noexcept {
    if (--weak_count == 0) {
      destroy_block();
    }
  }

  size_t shared_count{1};
  size_t weak_count{1};
};

template <typename U, typename Deleter = std::default_delete<U>, typename Alloc = std::allocator<U>>
struct control_block_regular final : base_control_block {
  control_block_regular(const Alloc& alloc = Alloc(), U* ptr = nullptr, Deleter&& del = Deleter())
      : ptr_(ptr)
      , del_(std::move(del))
      , alloc_(alloc) {}

  control_block_regular(const control_block_regular<U>& other)
      : control_block_regular(other.ptr_, other.del_, other.alloc_) {}

  void destroy_data() noexcept override {
    del_(ptr_);
  }

  void destroy_block() noexcept override {
    using block_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block_regular>;
    block_alloc_type block_alloc(alloc_);
    using block_traits = std::allocator_traits<block_alloc_type>;
    block_traits::deallocate(block_alloc, this, 1);
  }

  U* ptr_;
  [[no_unique_address]] Deleter del_;
  [[no_unique_address]] Alloc alloc_;
};

template <typename U, typename Alloc>
struct control_block_make_shared final : base_control_block {
  using alloc_object = typename std::allocator_traits<Alloc>::template rebind_alloc<U>;
  using alloc_traits_object = std::allocator_traits<alloc_object>;

  using alloc_block = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block_make_shared>;
  using alloc_traits_block = std::allocator_traits<alloc_block>;

  template <typename... Args>
  control_block_make_shared(const Alloc& alloc, Args&&... args)
      : alloc_(alloc) {
    alloc_traits_object::construct(alloc_, std::addressof(storage_.value_space_), std::forward<Args>(args)...);
  }

  void destroy_data() noexcept override {
    alloc_traits_object::destroy(alloc_, std::addressof(storage_.value_space_));
  }

  void destroy_block() noexcept override {
    delete this;
  }

  // storage of T value
  struct empty_byte {};

  template <typename Y, bool = std::is_trivially_destructible_v<U>>
  union storage {
    storage()
        : empty_{} {}

    empty_byte empty_;
    U value_space_;
  };

  template <typename Y>
  union storage<Y, false> {
    storage()
        : empty_{} {}

    empty_byte empty_;
    U value_space_;

    ~storage() {}
  };

  storage<U> storage_;
  [[no_unique_address]] Alloc alloc_;
};

struct move_ctor_t {
  explicit constexpr move_ctor_t() = default;
};

inline constexpr move_ctor_t move_ctor{};

template <typename BlockType, typename Alloc, typename... Args>
BlockType* allocate_block(Alloc&& alloc, Args&&... args) {
  using block_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<BlockType>;
  block_alloc_type block_alloc(alloc);

  BlockType* block_ptr = std::allocator_traits<block_alloc_type>::allocate(block_alloc, 1);

  using block_traits = std::allocator_traits<block_alloc_type>;

  try {
    block_traits::construct(block_alloc, block_ptr, std::forward<Alloc>(alloc), std::forward<Args>(args)...);
  } catch (...) {
    block_traits::deallocate(block_alloc, block_ptr, 1);
    throw;
  }
  return block_ptr;
}

} // namespace detail

/** @shared_ptr implementation
 * The main idea in @namespace detail
 * @tparam T value_stored
 * @param T* ptr_ @param detail::base_control_block* cb_
 */
template <typename T>
class shared_ptr {
  using base_control_block = detail::base_control_block;

  template <typename Y>
  using safe_conv = std::enable_if_t<std::is_convertible_v<Y*, T*>>;

  // It is design to provide access to the private control block
  template <typename Y>
  friend class weak_ptr;

  // It is made to provide access to the private control block
  template <typename Y>
  friend class shared_ptr;

  /**
   * Creating control_block with value in it
   * Should use it to create regular block bc some type made with no default construct
   *
   * Might throw exception while allocate and construct
   *
   * delete ptr is called if an exception occurs.
   */
  template <typename U, typename Deleter = std::default_delete<U>, typename Alloc = std::allocator<U>>
  auto create_regular_block(U* ptr, Deleter&& del = Deleter(), Alloc&& alloc = Alloc())
      -> detail::control_block_regular<U, Deleter, Alloc>* {
    using block_type = detail::control_block_regular<U, Deleter, Alloc>;

    block_type* control_block_ptr = nullptr;

    try {
      control_block_ptr =
          detail::allocate_block<block_type>(std::forward<Alloc>(alloc), ptr, std::forward<Deleter>(del));
    } catch (...) {
      del(ptr);
      throw;
    }

    return control_block_ptr;
  }

  template <typename Y>
  explicit shared_ptr(base_control_block* cb, Y* ptr)
      : ptr_(ptr)
      , cb_(cb) {
    if (cb_) {
      cb_->incr_shared();
    }
  }

  template <typename Y>
  explicit shared_ptr(detail::move_ctor_t, base_control_block*& cb, Y*& ptr)
      : ptr_(std::exchange(ptr, nullptr))
      , cb_(std::exchange(cb, nullptr)) {}

  template <typename Y, typename... Args>
  friend shared_ptr<Y> make_shared(Args&&... args);

public:
  shared_ptr() noexcept
      : ptr_(nullptr)
      , cb_(nullptr) {}

  explicit shared_ptr(std::nullptr_t) noexcept
      : shared_ptr() {}

  // try catch inside create_regular_block
  // delete ptr is called if an exception occurs.
  template <typename Y, typename = safe_conv<Y>>
  explicit shared_ptr(Y* ptr)
      : shared_ptr(ptr, std::default_delete<Y>()) {}

  // try catch inside create_regular_block
  // delete ptr is called if an exception occurs.
  template <typename Y, typename Deleter, typename = safe_conv<Y>>
  shared_ptr(Y* ptr, Deleter deleter)
      : ptr_(ptr)
      , cb_(create_regular_block(ptr, std::move(deleter))) {}

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other, T* ptr) noexcept
      : shared_ptr(other.cb_, ptr) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, T* ptr) noexcept
      : shared_ptr(detail::move_ctor, other.cb_, ptr) {
    other.ptr_ = nullptr;
  }

  shared_ptr(const shared_ptr& other) noexcept
      : shared_ptr(other.cb_, other.ptr_) {}

  template <typename Y, typename = safe_conv<Y>>
  shared_ptr(const shared_ptr<Y>& other) noexcept
      : shared_ptr(other.cb_, other.ptr_) {}

  template <typename Y, typename = safe_conv<Y>>
  explicit shared_ptr(const weak_ptr<Y>& other)
      : shared_ptr(other.cb_, other.ptr_) {}

  shared_ptr(shared_ptr&& other) noexcept
      : shared_ptr(detail::move_ctor, other.cb_, other.ptr_) {}

  template <typename Y, typename = safe_conv<Y>>
  shared_ptr(shared_ptr<Y>&& other) noexcept
      : shared_ptr(detail::move_ctor, other.cb_, other.ptr_) {}

  ~shared_ptr() {
    if (cb_) {
      cb_->decr_shared();
    }
  }

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    this->template operator= <T>(other);
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    shared_ptr(other).swap(*this);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    this->template operator= <T>(std::move(other));
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(shared_ptr<Y>&& other) {
    shared_ptr(std::move(other)).swap(*this);
    return *this;
  }

  T* get() const noexcept {
    return ptr_;
  }

  operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  T& operator*() const noexcept {
    return *ptr_;
  }

  T* operator->() const noexcept {
    return ptr_;
  }

  std::size_t use_count() const noexcept {
    return cb_ ? cb_->shared_count : 0;
  }

  void reset() noexcept {
    *this = shared_ptr();
  }

  // operator = throw exception
  // delete new_ptr is called if an exception occurs.
  template <typename Y>
  void reset(Y* new_ptr) {
    shared_ptr(new_ptr).swap(*this);
  }

  template <typename Y, typename Deleter>
  void reset(Y* new_ptr, Deleter deleter) {
    shared_ptr(new_ptr, std::move(deleter)).swap(*this);
  }

  void swap(shared_ptr& rhs) noexcept {
    std::swap(cb_, rhs.cb_);
    std::swap(ptr_, rhs.ptr_);
  }

  friend bool operator==(const shared_ptr& lhs, std::nullptr_t) noexcept {
    return !lhs;
  }

  friend bool operator==(std::nullptr_t, const shared_ptr& rhs) noexcept {
    return !rhs;
  }

  template <typename U>
  friend bool operator==(const shared_ptr& lhs, const shared_ptr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
  }

  friend bool operator!=(const shared_ptr& lhs, std::nullptr_t) noexcept {
    return lhs;
  }

  friend bool operator!=(std::nullptr_t, const shared_ptr& rhs) noexcept {
    return rhs;
  }

  template <typename U>
  friend bool operator!=(const shared_ptr& lhs, const shared_ptr<U>& rhs) noexcept {
    return lhs.get() != rhs.get();
  }

private:
  T* ptr_; // Contained pointer.
  base_control_block* cb_; // Reference counter.
};

/** @weak_ptr implementation
 * The main idea in @namespace detail
 * @tparam T value_stored
 * @param detail::base_control_block* cb_
 */
template <typename T>
class weak_ptr {
  using base_control_block = detail::base_control_block;

  template <typename Y>
  using safe_conv = std::enable_if_t<std::is_convertible_v<Y*, T*>>;

  template <typename Y>
  friend class weak_ptr;

  template <typename Y>
  friend class shared_ptr;

  explicit weak_ptr(base_control_block* cb, T* ptr)
      : ptr_(ptr)
      , cb_(cb) {
    if (cb_) {
      cb_->incr_weak();
    }
  }

public:
  weak_ptr() noexcept
      : ptr_(nullptr)
      , cb_(nullptr) {}

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr(const shared_ptr<Y>& other_shared) noexcept
      : weak_ptr(other_shared.cb_, other_shared.ptr_) {}

  weak_ptr(const weak_ptr& other) noexcept
      : weak_ptr(other.cb_, other.ptr_) {}

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr(const weak_ptr<Y>& other) noexcept
      : weak_ptr(other.cb_, other.ptr_) {}

  weak_ptr(weak_ptr&& other) noexcept
      : ptr_(other.ptr_)
      , cb_(other.cb_) {
    other.ptr_ = nullptr;
    other.cb_ = nullptr;
  }

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr(weak_ptr<Y>&& other) noexcept
      : ptr_(other.ptr_)
      , cb_(other.cb_) {
    other.ptr_ = nullptr;
    other.cb_ = nullptr;
  }

  ~weak_ptr() {
    reset();
  }

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(const weak_ptr& other) noexcept {
    this->template operator= <T>(other);
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(const weak_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    this->template operator= <T>(std::move(other));
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(weak_ptr<Y>&& other) noexcept {
    weak_ptr(std::move(other)).swap(*this);
    return *this;
  }

  shared_ptr<T> lock() const noexcept {
    return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
  }

  bool expired() const noexcept {
    return !cb_ || cb_->shared_count == 0;
  }

  void reset() noexcept {
    if (cb_) {
      cb_->decr_weak();
      cb_ = nullptr;
    }
  }

  void swap(weak_ptr& rhs) noexcept {
    std::swap(cb_, rhs.cb_);
    std::swap(ptr_, rhs.ptr_);
  }

private:
  T* ptr_; // Contained pointer.
  base_control_block* cb_; // Reference counter.
};

/**
 *  template <typename T, typename... Args>
 *  shared_ptr<T> detail::make_shared(Args&&... args)
 */

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  using block_type = detail::control_block_make_shared<T, std::allocator<T>>;
  block_type* control_block = detail::allocate_block<block_type>(std::allocator<T>(), std::forward<Args>(args)...);

  control_block->shared_count = 0;
  return shared_ptr<T>(control_block, std::addressof(control_block->storage_.value_space_));
}
