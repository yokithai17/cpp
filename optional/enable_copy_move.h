#pragma once

namespace detail {
template <bool CopyConstruct, bool CopyAssign, bool MoveConstruct, bool MoveAssign>
struct enable_copy_move;

template <>
struct enable_copy_move<true, true, true, true> {
  constexpr enable_copy_move() = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<false, true, true, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<true, false, true, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<false, false, true, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<true, true, false, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<false, true, false, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<true, false, false, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<false, false, false, true> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = default;
};

template <>
struct enable_copy_move<true, true, true, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<false, true, true, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<true, false, true, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<false, false, true, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = default;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<true, true, false, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<false, true, false, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<true, false, false, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = default;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};

template <>
struct enable_copy_move<false, false, false, false> {
  constexpr enable_copy_move() noexcept = default;

  constexpr enable_copy_move(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move(enable_copy_move&&) noexcept = delete;

  constexpr enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;

  constexpr enable_copy_move& operator=(enable_copy_move&&) noexcept = delete;
};
} // namespace detail
