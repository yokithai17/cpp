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

  virtual void* get_pointer() noexcept = 0;

  virtual void destroy_data() noexcept = 0;

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
      delete this;
    }
  }

  size_t shared_count{1};
  size_t weak_count{1};
};

template <typename U, typename Deleter = std::default_delete<U>, typename Alloc = std::allocator<U>>
struct control_block_regular final : base_control_block {
  control_block_regular(U* ptr, Deleter&& del = Deleter(), const Alloc& alloc = Alloc())
      : ptr_(ptr)
      , del_(std::move(del))
      , alloc_(alloc) {}

  control_block_regular(const control_block_regular<U>& other)
      : ptr_(other.ptr_)
      , del_(other.del_)
      , alloc_(other.alloc_) {}

  void* get_pointer() noexcept override {
    return ptr_;
  }

  void destroy_data() noexcept override {
    del_(ptr_);
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
    alloc_traits_object::construct(alloc_, reinterpret_cast<U*>(&value_space_), std::forward<Args>(args)...);
  }

  void* get_pointer() noexcept override {
    return &value_space_;
  }

  void destroy_data() noexcept override {
    alloc_traits_object::destroy(alloc_, reinterpret_cast<U*>(&value_space_));
  }

  // storage of T value
  std::aligned_storage_t<sizeof(U), std::alignment_of_v<U>> value_space_;
  [[no_unique_address]] Alloc alloc_;
};

template <typename T, typename Alloc, typename... Args>
shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args) {
  using control_block_type = control_block_make_shared<T, Alloc>;

  using block_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block_type>;

  block_alloc_type block_alloc = alloc;
  using block_traits = std::allocator_traits<block_alloc_type>;
  control_block_type* control_block = nullptr;

  try {
    control_block = block_traits::allocate(block_alloc, 1);
    block_traits::construct(block_alloc, control_block, alloc, std::forward<Args>(args)...);
  } catch (...) {
    block_traits::deallocate(block_alloc, control_block, 1);
    throw;
  }

  auto block = static_cast<base_control_block*>(control_block);
  return shared_ptr<T>(block);
}

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return detail::allocate_shared<T, std::allocator<T>>(std::allocator<T>(), std::forward<Args>(args)...);
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

  // provide access to the private constructor shared_ptr(control_block_ptr)
  template <typename U, typename Alloc, typename... Args>
  friend shared_ptr<U> detail::allocate_shared(const Alloc& alloc, Args&&... args);

  explicit shared_ptr(base_control_block* cb)
      : cb_(cb) {
    if (cb_) {
      ptr_ = static_cast<T*>(cb_->get_pointer());
    }
  }

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
    using block_alloc_type =
        typename std::allocator_traits<Alloc>::template rebind_alloc<detail::control_block_regular<U, Deleter, Alloc>>;
    block_alloc_type block_alloc(alloc);

    detail::control_block_regular<U, Deleter, Alloc>* control_block_ptr = nullptr;
    try {
      control_block_ptr = std::allocator_traits<block_alloc_type>::allocate(block_alloc, 1);

      std::allocator_traits<block_alloc_type>::construct(
          block_alloc,
          control_block_ptr,
          ptr,
          std::forward<Deleter>(del),
          std::forward<Alloc>(alloc)
      );
    } catch (...) {
      del(ptr);
      std::allocator_traits<block_alloc_type>::deallocate(block_alloc, control_block_ptr, 1);
      throw;
    }

    return control_block_ptr;
  }

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
      : ptr_(ptr)
      , cb_(create_regular_block(ptr)) {}

  // try catch inside create_regular_block
  // delete ptr is called if an exception occurs.
  template <typename Y, typename Deleter, typename = safe_conv<Y>>
  shared_ptr(Y* ptr, Deleter deleter)
      : ptr_(ptr)
      , cb_(create_regular_block(ptr, std::move(deleter))) {}

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other, T* ptr) noexcept
      : ptr_(ptr)
      , cb_(other.cb_) {
    if (cb_) {
      cb_->incr_shared();
    }
  }

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, T* ptr) noexcept
      : ptr_(std::exchange(ptr, nullptr))
      , cb_(std::exchange(other.cb_, nullptr)) {
    other.ptr_ = nullptr;
  }

  shared_ptr(const shared_ptr& other) noexcept
      : ptr_(other.ptr_)
      , cb_(other.cb_) {
    if (cb_) {
      cb_->incr_shared();
    }
  }

  template <typename Y, typename = safe_conv<Y>>
  shared_ptr(const shared_ptr<Y>& other) noexcept
      : ptr_(other.ptr_)
      , cb_(other.cb_) {
    if (cb_) {
      cb_->incr_shared();
    }
  }

  template <typename Y, typename = safe_conv<Y>>
  explicit shared_ptr(const weak_ptr<Y>& other)
      : ptr_(other.ptr_)
      , cb_(other.cb_) {
    if (cb_) {
      cb_->incr_shared();
    }
  }

  shared_ptr(shared_ptr&& other) noexcept
      : ptr_(std::exchange(other.ptr_, nullptr))
      , cb_(std::exchange(other.cb_, nullptr)) {}

  template <typename Y, typename = safe_conv<Y>>
  shared_ptr(shared_ptr<Y>&& other) noexcept
      : ptr_(std::exchange(other.ptr_, nullptr))
      , cb_(std::exchange(other.cb_, nullptr)) {}

  ~shared_ptr() {
    if (cb_) {
      cb_->decr_shared();
    }
  }

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    shared_ptr(other).swap(*this);
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    shared_ptr(other).swap(*this);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    shared_ptr(std::move(other)).swap(*this);
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
    if (cb_) {
      cb_->decr_shared();
      cb_ = nullptr;
      ptr_ = nullptr;
    }
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

public:
  weak_ptr() noexcept
      : ptr_(nullptr)
      , cb_(nullptr) {}

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr(const shared_ptr<Y>& other_shared) noexcept
      : ptr_(other_shared.ptr_)
      , cb_(other_shared.cb_) {
    if (cb_) {
      cb_->incr_weak();
    }
  }

  weak_ptr(const weak_ptr& other) noexcept
      : ptr_{other.ptr_}
      , cb_(other.cb_) {
    if (cb_) {
      cb_->incr_weak();
    }
  }

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr(const weak_ptr<Y>& other) noexcept
      : ptr_(other.ptr_)
      , cb_(other.cb_) {
    if (cb_) {
      cb_->incr_weak();
    }
  }

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
    if (cb_) {
      cb_->decr_weak();
    }
  }

  template <typename Y, typename = safe_conv<Y>>
  weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(const weak_ptr& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(const weak_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    weak_ptr(std::move(other)).swap(*this);
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

using detail::make_shared;
