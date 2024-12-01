#pragma once

#include "enable_copy_move.h"

#include <compare>
#include <memory>
#include <type_traits>

struct in_place_t {
  explicit constexpr in_place_t() = default;
};

inline constexpr in_place_t in_place{};

struct nullopt_t {
  enum class dummy {
    DUMMY
  };

  explicit constexpr nullopt_t(dummy) noexcept {}
};

inline constexpr nullopt_t nullopt{nullopt_t::dummy::DUMMY};

namespace detail {
/**
***************************************************************************************************
********************              OPTIONAL STORAGE              ***********************************
***************************************************************************************************
*/

struct not_triv_t {};

inline constexpr not_triv_t not_triv{};

template <typename T>
struct optional_storage_base {
  constexpr optional_storage_base() noexcept = default;

  constexpr optional_storage_base(const optional_storage_base&) noexcept = default;

  constexpr optional_storage_base(optional_storage_base&&) noexcept = default;

  constexpr optional_storage_base& operator=(const optional_storage_base&) noexcept = default;

  constexpr optional_storage_base& operator=(optional_storage_base&&) noexcept = default;

  constexpr T* get_pointer() noexcept {
    return std::addressof(this->storage_.value_);
  }

  template <typename... Args>
  explicit constexpr optional_storage_base(in_place_t, Args&&... args) {
    this->construct(std::forward<Args>(args)...);
  }

  constexpr optional_storage_base(not_triv_t, const optional_storage_base& other) {
    if (other.has_value_) {
      this->construct(other.get());
    }
  }

  constexpr optional_storage_base(not_triv_t, optional_storage_base&& other) {
    if (other.has_value_) {
      this->construct(std::move(other.get()));
    }
  }

  constexpr void copy_assign(const optional_storage_base& other) {
    if (this->has_value_ && other.has_value_) {
      this->get() = other.get();
    } else if (other.has_value_) {
      this->construct(other.get());
    } else {
      this->destroy();
    }
  }

  constexpr void move_assign(optional_storage_base&& other) {
    if (this->has_value_ && other.has_value_) {
      this->get() = std::move(other.get());
    } else if (other.has_value_) {
      this->construct(std::move(other.get()));
    } else {
      this->destroy();
    }
  }

  template <typename... Args>
  constexpr void construct(Args&&... args) {
    std::construct_at(get_pointer(), std::forward<Args>(args)...);
    has_value_ = true;
  }

  constexpr void destroy() noexcept {
    if (has_value_) {
      std::destroy_at(get_pointer());
      has_value_ = false;
    }
  }

  constexpr const T& get() const {
    return this->storage_.value_;
  }

  constexpr T& get() {
    return this->storage_.value_;
  }

  constexpr bool has_value() const noexcept {
    return has_value_;
  }

  template <typename U, bool = std::is_trivially_destructible_v<U>>
  union storage {
    constexpr storage() noexcept
        : empty_() {}

    struct empty_type {};

    empty_type empty_;
    T value_;
  };

  template <typename U>
  union storage<U, false> {
    constexpr storage() noexcept
        : empty_() {}

    constexpr ~storage() {}

    struct empty_type {};

    empty_type empty_;
    T value_;
  };

  storage<T> storage_{};
  bool has_value_{false};
};

/**
***************************************************************************************************
********************              OPTIONAL TRIVIALLY ASSIGN     ***********************************
***************************************************************************************************
*/

template <
    typename T,
    bool = std::is_trivially_destructible_v<T>,
    bool = std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>,
    bool = std::is_trivially_move_assignable_v<T> && std::is_trivially_move_constructible_v<T>>
struct optional_storage : optional_storage_base<T> {
  using optional_storage_base<T>::optional_storage_base;
};

template <typename T>
struct optional_storage<T, true, false, false> : optional_storage_base<T> {
  using optional_storage_base<T>::optional_storage_base;

  constexpr optional_storage() = default;

  constexpr optional_storage(const optional_storage&) = default;

  constexpr optional_storage(optional_storage&&) = default;

  constexpr optional_storage& operator=(const optional_storage& other
  ) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>) {
    this->copy_assign(other);
    return *this;
  }

  constexpr optional_storage& operator=(optional_storage&& other
  ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
    this->move_assign(std::move(other));
    return *this;
  }
};

template <typename T>
struct optional_storage<T, true, true, false> : optional_storage_base<T> {
  using optional_storage_base<T>::optional_storage_base;

  constexpr optional_storage() = default;

  constexpr optional_storage(const optional_storage&) = default;

  constexpr optional_storage(optional_storage&&) = default;

  constexpr optional_storage& operator=(const optional_storage& other) = default;

  constexpr optional_storage& operator=(optional_storage&& other
  ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
    this->move_assign(std::move(other));
    return *this;
  }
};

template <typename T>
struct optional_storage<T, true, false, true> : optional_storage_base<T> {
  using optional_storage_base<T>::optional_storage_base;

  constexpr optional_storage() noexcept = default;

  constexpr optional_storage(const optional_storage&) noexcept = default;

  constexpr optional_storage(optional_storage&&) noexcept = default;

  constexpr optional_storage& operator=(const optional_storage& other
  ) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>) {
    this->copy_assign(other);
    return *this;
  }

  constexpr optional_storage& operator=(optional_storage&& other) = default;
};

template <typename T, bool _Copy, bool _Move>
struct optional_storage<T, false, _Copy, _Move> : optional_storage<T, true, false, false> {
  using optional_storage<T, true, false, false>::optional_storage;

  constexpr ~optional_storage() {
    this->destroy();
  }

  constexpr optional_storage() = default;

  constexpr optional_storage(const optional_storage&) = default;

  constexpr optional_storage(optional_storage&&) = default;

  constexpr optional_storage& operator=(const optional_storage& other) = default;

  constexpr optional_storage& operator=(optional_storage&& other) = default;
};

/**
 **************************************************************************************************
 ********************              OPTIONAL TRIV MOVE AND COPY        *****************************
 ********************                 OPTIONAL BASE                   *****************************
 **************************************************************************************************
 */

template <
    typename T,
    bool = std::is_trivially_copy_constructible_v<T>,
    bool = std::is_trivially_move_constructible_v<T>>
struct optional_base {
  constexpr optional_base() = default;

  constexpr optional_base(const optional_base& other) = default;

  constexpr optional_base(optional_base&& other) = default;

  constexpr optional_base& operator=(const optional_base& other) = default;

  constexpr optional_base& operator=(optional_base&& other) = default;

  template <typename... Args>
  explicit constexpr optional_base(in_place_t, Args&&... args)
      : storage_(in_place, std::forward<Args>(args)...) {}

  optional_storage<T> storage_;
};

template <typename T>
struct optional_base<T, false, false> {
  constexpr optional_base() = default;

  template <typename... Args>
  constexpr explicit optional_base(in_place_t, Args&&... args)
      : storage_(in_place, std::forward<Args>(args)...) {}

  constexpr optional_base(const optional_base& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
      : storage_(not_triv, other.storage_) {}

  constexpr optional_base(optional_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
      : storage_(not_triv, std::move(other.storage_)) {}

  constexpr optional_base& operator=(const optional_base& other) = default;

  constexpr optional_base& operator=(optional_base&& other) = default;

  optional_storage<T> storage_;
};

template <typename T>
struct optional_base<T, true, false> {
  constexpr optional_base() noexcept = default;

  constexpr optional_base(const optional_base& other) = default;

  constexpr optional_base(optional_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
      : storage_(not_triv, std::move(other.storage_)) {}

  constexpr optional_base& operator=(const optional_base& other) = default;

  constexpr optional_base& operator=(optional_base&& other) = default;

  template <typename... Args>
  explicit constexpr optional_base(in_place_t, Args&&... args)
      : storage_(in_place, std::forward<Args>(args)...) {}

  optional_storage<T> storage_;
};

template <typename T>
struct optional_base<T, false, true> {
  constexpr optional_base() = default;

  constexpr optional_base(const optional_base& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
      : storage_(not_triv, other.storage_) {}

  constexpr optional_base(optional_base&& other) = default;

  constexpr optional_base& operator=(const optional_base& other) = default;

  constexpr optional_base& operator=(optional_base&& other) = default;

  template <typename... Args>
  explicit constexpr optional_base(in_place_t, Args&&... args)
      : storage_(in_place, std::forward<Args>(args)...) {}

  template <typename... Args>
  constexpr void construct(Args&&... args) {
    storage_.construct(std::forward<Args>(args)...);
  }

  optional_storage<T> storage_;
};
} // namespace detail

/**
***************************************************************************************************
********************              OPTIONAL                      ***********************************
***************************************************************************************************
*/

template <typename T>
class optional
    : detail::optional_base<T>
    , detail::enable_copy_move<
          std::is_copy_constructible_v<T>,
          std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>,
          std::is_move_constructible_v<T>,
          std::is_move_assignable_v<T> && std::is_move_constructible_v<T>> {
  using base_type = detail::optional_base<T>;

public:
  using detail::optional_base<T>::optional_base;

  using value_type = T;

  constexpr optional(nullopt_t) noexcept
      : optional() {}

  template <
      typename U = T,
      typename = std::enable_if_t<
          std::is_constructible_v<T, U> && !std::is_same_v<std::remove_cvref_t<U>, in_place_t> &&
          !std::is_same_v<optional, std::remove_cvref_t<U>>>>
  constexpr explicit(!std::is_convertible_v<U, T>) optional(U&& value) noexcept(std::is_nothrow_constructible_v<T, U>)
      : base_type(in_place, std::forward<U>(value)) {}

  template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
  constexpr explicit optional(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : base_type(in_place, std::forward<Args>(args)...) {}

  constexpr optional& operator=(nullopt_t) noexcept {
    this->reset();
    return *this;
  }

  template <
      typename U = T,
      typename = std::enable_if_t<
          !std::is_same_v<optional, std::remove_cvref_t<U>> &&
          !(std::is_scalar_v<T> && std::is_same_v<T, std::remove_cvref_t<U>>) && std::is_constructible_v<T, U> &&
          std::is_assignable_v<T&, U>>>
  constexpr optional& operator=(U&& value
  ) noexcept(std::is_nothrow_constructible_v<T, U> && std::is_nothrow_assignable_v<T&, U>) {
    if (this->has_value()) {
      this->storage_.get() = std::forward<U>(value);
    } else {
      this->storage_.construct(std::forward<U>(value));
    }
    return *this;
  }

  constexpr explicit operator bool() const noexcept {
    return this->storage_.has_value();
  }

  constexpr T& operator*() & noexcept {
    return this->storage_.get();
  }

  constexpr const T& operator*() const& noexcept {
    return this->storage_.get();
  }

  constexpr T&& operator*() && noexcept {
    return std::move(this->storage_.get());
  }

  constexpr const T&& operator*() const&& noexcept {
    return std::move(this->storage_.get());
  }

  constexpr T* operator->() noexcept {
    return std::addressof(this->storage_.get());
  }

  constexpr const T* operator->() const noexcept {
    return std::addressof(this->storage_.get());
  }

  constexpr void reset() {
    this->storage_.destroy();
  }

  constexpr bool has_value() const noexcept {
    return this->storage_.has_value();
  }

  template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
  constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    this->reset();
    this->storage_.construct(std::forward<Args>(args)...);
    return this->storage_.get();
  }
};

template <typename T>
inline constexpr std::enable_if_t<std::is_move_constructible_v<T> && std::is_swappable_v<T>> swap(
    optional<T>& lhs,
    optional<T>& rhs
) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
  using std::swap;
  if (lhs.has_value() && rhs.has_value()) {
    swap(*lhs, *rhs);
  } else if (lhs.has_value()) {
    rhs.emplace(std::move(*lhs));
    lhs.reset();
  } else if (rhs.has_value()) {
    lhs.emplace(std::move(*rhs));
    rhs.reset();
  }
}

template <typename T>
std::enable_if_t<!(std::is_move_constructible_v<T> && std::is_swappable_v<T>)>
swap(optional<T>&, optional<T>&) = delete;

template <typename T>
constexpr bool operator==(const optional<T>& lhs, const optional<T>& rhs) {
  if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
    return false;
  }
  if (!static_cast<bool>(lhs)) {
    return true;
  }
  return *lhs == *rhs;
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const optional<T>& rhs) {
  if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
    return true;
  }

  if (!static_cast<bool>(lhs)) {
    return false;
  }

  return *lhs != *rhs;
}

template <typename T>
constexpr bool operator<(const optional<T>& lhs, const optional<T>& rhs) {
  if (!static_cast<bool>(rhs)) {
    return false;
  }
  if (!static_cast<bool>(lhs)) {
    return true;
  }
  return *lhs < *rhs;
}

template <typename T>
constexpr bool operator<=(const optional<T>& lhs, const optional<T>& rhs) {
  if (!static_cast<bool>(lhs)) {
    return true;
  }

  if (!static_cast<bool>(rhs)) {
    return false;
  }

  return *lhs <= *rhs;
}

template <typename T>
constexpr bool operator>(const optional<T>& lhs, const optional<T>& rhs) {
  if (!static_cast<bool>(lhs)) {
    return false;
  }
  if (!static_cast<bool>(rhs)) {
    return true;
  }

  return *lhs > *rhs;
}

template <typename T>
constexpr bool operator>=(const optional<T>& lhs, const optional<T>& rhs) {
  if (!static_cast<bool>(rhs)) {
    return true;
  }

  if (!static_cast<bool>(lhs)) {
    return false;
  }

  return *lhs >= *rhs;
}

template <class T>
constexpr std::compare_three_way_result_t<T> operator<=>(const optional<T>& lhs, const optional<T>& rhs) {
  return lhs.has_value() && rhs.has_value() ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
}

template <typename T>
optional(T) -> optional<T>;
