#pragma once

#include <algorithm>
#include <cstddef>
#include <exception>
#include <string>
#include <typeinfo>
#include <utility>

class bad_function_call : public std::exception {
public:
  bad_function_call() = default;

  bad_function_call(const char* msg)
      : message(msg) {}

  ~bad_function_call() = default;

  const char* what() const noexcept override {
    return message.c_str();
  }

private:
  std::string message;
};

template <typename F>
class function;

template <typename Ret, typename... Args>
class function<Ret(Args...)> {
  struct storage_t;
  using invoker_ptr_t = Ret (*)(void*, Args&&...);
  using destroy_ptr_t = void (*)(void*);
  using copy_ptr_t = void (*)(void*, void*&, storage_t&);
  using move_copy_ptr_t = void (*)(void*, storage_t&, storage_t*);

  static constexpr std::size_t BUFFER_SIZE = sizeof(void*) * 4;

  template <typename F>
  static constexpr bool can_use_small_buffer = sizeof(F) <= BUFFER_SIZE && std::is_nothrow_move_constructible_v<F> &&
                                               std::alignment_of_v<F> <= std::alignment_of_v<std::max_align_t>;

  struct operations_t {
    operations_t() = default;

    ~operations_t() = default;

    operations_t(destroy_ptr_t d_ptr, copy_ptr_t c_ptr, const std::type_info* t_info_ptr, move_copy_ptr_t m_ptr)
        : destroy_ptr(d_ptr)
        , copy_ptr(c_ptr)
        , type_info_ptr(t_info_ptr)
        , move_copy_ptr(m_ptr) {}

    operations_t(const operations_t&) = default;

    operations_t& operator=(const operations_t&) = default;

    destroy_ptr_t destroy_ptr{};
    copy_ptr_t copy_ptr;
    const std::type_info* type_info_ptr{};
    move_copy_ptr_t move_copy_ptr;
  };

  struct storage_t {
    storage_t() = default;

    ~storage_t() = default;

    storage_t(const storage_t& other) {
      std::copy_n(other.buffer_, BUFFER_SIZE, buffer_);
    }

    storage_t& operator=(const storage_t& other) {
      std::copy_n(other.buffer_, BUFFER_SIZE, buffer_);
      return *this;
    }

    alignas(std::max_align_t) unsigned char buffer_[BUFFER_SIZE];
  };

  template <typename F>
  static void destructor(void* fptr) {
    if constexpr (can_use_small_buffer<F>) {
      (*static_cast<F*>(fptr)).~F();
    } else {
      delete static_cast<F*>(fptr);
    }
  }

  template <typename F>
  static Ret invoker(void* fptr, Args&&... args) {
    return (*static_cast<F*>(fptr))(std::forward<Args>(args)...);
  }

  template <typename F>
  static void copy(void* from_void_ptr, void*& to_void_ptr, storage_t& storage) {
    F* from_fptr = static_cast<F*>(from_void_ptr);
    F*& to_fptr = reinterpret_cast<F*&>(to_void_ptr);

    if constexpr (can_use_small_buffer<F>) {
      new (storage.buffer_) F(*from_fptr);
      to_fptr = reinterpret_cast<F*>(storage.buffer_);
    } else {
      F* temp_fptr = new F(*from_fptr);
      to_fptr = temp_fptr;
    }
  }

  template <typename F>
  static void move_copy(void*, storage_t& from_storage, storage_t* to_storage_ptr) noexcept {
    if constexpr (can_use_small_buffer<F>) {
      new (to_storage_ptr) storage_t(from_storage);
    }
  }

  void swap(function& other) noexcept {
    std::swap(storage_, other.storage_);
    std::swap(operations_, other.operations_);
    std::swap(invoker_ptr_, other.invoker_ptr_);
    std::swap(fptr_, other.fptr_);
  }

public:
  function() noexcept = default;

  template <typename F>
  function(F func)
      : operations_(
            reinterpret_cast<destroy_ptr_t>(&destructor<F>),
            reinterpret_cast<copy_ptr_t>(&copy<F>),
            &typeid(F),
            reinterpret_cast<move_copy_ptr_t>(&move_copy<F>)
        )
      , invoker_ptr_(reinterpret_cast<invoker_ptr_t>(&invoker<F>)) {
    if constexpr (can_use_small_buffer<F>) {
      new (storage_.buffer_) F(std::move(func));
      fptr_ = storage_.buffer_;
    } else {
      fptr_ = new F(std::move(func));
    }
  }

  function(const function& other)
      : fptr_(nullptr)
      , operations_(other.operations_)
      , invoker_ptr_(other.invoker_ptr_) {
    if (other.fptr_) {
      operations_.copy_ptr(other.fptr_, fptr_, storage_);
    }
  }

  function(function&& other) noexcept
      : fptr_(std::exchange(other.fptr_, nullptr))
      , operations_(std::move(other.operations_))
      , invoker_ptr_(std::exchange(other.invoker_ptr_, nullptr)) {
    if (fptr_) {
      operations_.move_copy_ptr(fptr_, other.storage_, &storage_);
    }
  }

  function& operator=(const function& other) {
    if (this != &other) {
      operations_t& prev_operations_ = operations_;
      void* prev_fptr = fptr_;

      try {
        if (other.fptr_) {
          other.operations_.copy_ptr(other.fptr_, fptr_, storage_);
          operations_ = other.operations_;
          invoker_ptr_ = other.invoker_ptr_;
        } else {
          fptr_ = nullptr;
        }

        if (prev_fptr) {
          prev_operations_.destroy_ptr(prev_fptr);
        }
      } catch (...) {
        operations_ = prev_operations_;
        fptr_ = prev_fptr;
        throw;
      }
    }
    return *this;
  }

  function& operator=(function&& other) noexcept {
    if (this != &other) {
      if (fptr_) {
        operations_.destroy_ptr(fptr_);
        fptr_ = nullptr;
      }

      if (other.fptr_) {
        fptr_ = std::exchange(other.fptr_, nullptr);
        operations_ = std::move(other.operations_);
        storage_ = std::move(other.storage_);
        invoker_ptr_ = std::exchange(other.invoker_ptr_, nullptr);
        other.fptr_ = nullptr;
      }
    }
    return *this;
  }

  ~function() {
    if (fptr_) {
      operations_.destroy_ptr(fptr_);
    }
  }

  explicit operator bool() const noexcept {
    return fptr_ != nullptr;
  }

  Ret operator()(Args... args) const {
    if (!fptr_) {
      throw bad_function_call();
    }
    return invoker_ptr_(fptr_, std::forward<Args>(args)...);
  }

  template <typename F>
  F* target() noexcept {
    if (!fptr_ || operations_.type_info_ptr != &typeid(F)) {
      return nullptr;
    }
    return static_cast<F*>(fptr_);
  }

  template <typename F>
  const F* target() const noexcept {
    if (!fptr_ || operations_.type_info_ptr != &typeid(F)) {
      return nullptr;
    }
    return static_cast<const F*>(fptr_);
  }

private:
  void* fptr_{nullptr};
  storage_t storage_{};

  operations_t operations_{};
  invoker_ptr_t invoker_ptr_{nullptr};
};
