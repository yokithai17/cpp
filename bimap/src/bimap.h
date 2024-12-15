#pragma once

#include "intrusive_map.h"

#include <cstddef>
#include <stdexcept>
#include <type_traits>

template <
    typename Left,
    typename Right,
    typename CompareLeft = std::less<Left>,
    typename CompareRight = std::less<Right>>
class bimap {
  using size_type = std::size_t;

  using node_t = intrusive::bimap_element<Left, Right>;
  using left_node_t = intrusive::map_element<true, Left>;
  using right_node_t = intrusive::map_element<false, Right>;
  using base_node_t = intrusive::map_element_base;

  intrusive::intrusive_map<true, Left, CompareLeft> map_left;
  intrusive::intrusive_map<false, Right, CompareRight> map_right;

  size_type size_{0};

  template <bool IsLeft>
  static base_node_t* node_to_base(node_t* raw_node) {
    if constexpr (IsLeft) {
      return static_cast<base_node_t*>(static_cast<left_node_t*>(raw_node));
    } else {
      return static_cast<base_node_t*>(static_cast<right_node_t*>(raw_node));
    }
  }

  template <bool IsLeft>
  static node_t* base_to_node(base_node_t* node) {
    if constexpr (IsLeft) {
      return static_cast<node_t*>(static_cast<left_node_t*>(node));
    } else {
      return static_cast<node_t*>(static_cast<right_node_t*>(node));
    }
  }

public:
  using left_iterator = intrusive::based_iterator<Left, Right, true>;
  using right_iterator = intrusive::based_iterator<Left, Right, false>;

  explicit bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight()) noexcept
      : map_left(std::move(compare_left))
      , map_right(std::move(compare_right)) {
    map_left.connect_map(map_right.end());
    map_right.connect_map(map_left.end());
  }

  bimap(const bimap& other)
      : bimap(other.map_left.comp, other.map_right.comp) {
    try {
      for (auto it = other.begin_left(); it != other.end_left(); ++it) {
        insert(*it, *it.flip());
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  bimap(bimap&& other) noexcept
      : map_left(std::move(other.map_left))
      , map_right(std::move(other.map_right))
      , size_(other.size_) {
    other.size_ = 0;
  }

  bimap& operator=(const bimap& other) {
    if (this != &other) {
      bimap(other).swap(*this);
    }
    return *this;
  }

  bimap& operator=(bimap&& other) noexcept {
    if (this != &other) {
      bimap(std::move(other)).swap(*this);
    }
    return *this;
  }

  ~bimap() {
    clear();
  }

  void clear() {
    erase_left(this->begin_left(), this->end_left());
  }

  template <typename L = Left, typename R = Right>
  left_iterator insert(L&& left, R&& right) {
    if (find_left(left) != this->end_left() || find_right(right) != this->end_right()) {
      return this->end_left();
    }

    auto* raw_node = new node_t(std::forward<L>(left), std::forward<R>(right));
    base_node_t* res = nullptr;
    try {
      auto* left_node = node_to_base<true>(raw_node);
      res = map_left.insert(left_node);
    } catch (...) {
      delete raw_node;
      throw;
    }

    try {
      auto* right_node = node_to_base<false>(raw_node);
      map_right.insert(right_node);
    } catch (...) {
      map_left.erase(res);
      delete raw_node;
      throw;
    }
    ++size_;
    return left_iterator(res);
  }

  void erase_(base_node_t* left_node, base_node_t* right_node) {
    map_left.erase(left_node);
    map_right.erase(right_node);
    auto* raw_node = base_to_node<true>(left_node);
    delete raw_node;
    --size_;
  }

  left_iterator erase_left(left_iterator it) {
    auto del = it++;
    erase_(del.get_data(), del.flip().get_data());
    return it;
  }

  right_iterator erase_right(right_iterator it) {
    auto del = it++;
    erase_(del.flip().get_data(), del.get_data());
    return it;
  }

  bool erase_left(const Left& left) {
    if (auto it = find_left(left); it != this->end_left()) {
      erase_left(it);
      return true;
    }
    return false;
  }

  bool erase_right(const Right& right) {
    if (auto it = find_right(right); it != this->end_right()) {
      erase_right(it);
      return true;
    }
    return false;
  }

  left_iterator erase_left(left_iterator first, left_iterator last) {
    while (first != last) {
      first = erase_left(first);
    }
    return last;
  }

  right_iterator erase_right(right_iterator first, right_iterator last) {
    while (first != last) {
      first = erase_right(first);
    }
    return last;
  }

  left_iterator find_left(const Left& left) const {
    return left_iterator(map_left.find(left));
  }

  right_iterator find_right(const Right& right) const {
    return right_iterator(map_right.find(right));
  }

  const Right& at_left(const Left& key) const {
    auto it = find_left(key);
    if (it != this->end_left()) {
      return *it.flip();
    }
    throw std::out_of_range("");
  }

  const Left& at_right(const Right& key) const {
    auto it = find_right(key);
    if (it != this->end_right()) {
      return *it.flip();
    }
    throw std::out_of_range("");
  }

  const Right& at_left_or_default(const Left& key)
    requires (std::is_default_constructible_v<Right>)
  {
    left_iterator left_it(map_left.find(key));
    if (left_it != end_left()) {
      return *left_it.flip();
    }

    Right r{};
    right_iterator replace_right(map_right.find(r));
    if (replace_right == end_right()) {
      return *insert(key, std::move(r)).flip();
    }

    auto* row_node = new node_t(key, std::move(r));

    auto right_node = node_to_base<false>(row_node);
    auto left_node = node_to_base<true>(row_node);

    try {
      map_left.insert(left_node);
      map_left.erase(replace_right.flip().get_data());
      replace_right.get_data()->replace(right_node);
      delete base_to_node<false>(replace_right.get_data());
      return static_cast<right_node_t*>(row_node)->get();
    } catch (...) {
      delete row_node;
      throw;
    }
  }

  const Left& at_right_or_default(const Right& key)
    requires (std::is_default_constructible_v<Left>)
  {
    right_iterator right_it(map_right.find(key));
    if (right_it != end_right()) {
      return *right_it.flip();
    }

    Left l{};
    left_iterator replace_left(map_left.find(l));
    if (replace_left == end_left()) {
      return *insert(std::move(l), key);
    }

    auto* row_node = new node_t(std::move(l), key);

    auto left_node = node_to_base<true>(row_node);
    auto right_node = node_to_base<false>(row_node);

    try {
      map_right.insert(right_node);
      map_right.erase(replace_left.flip().get_data());
      replace_left.get_data()->replace(left_node);
      delete base_to_node<true>(replace_left.get_data());
      return static_cast<left_node_t*>(row_node)->get();
    } catch (...) {
      delete row_node;
      throw;
    }
  }

  left_iterator lower_bound_left(const Left& left) const {
    return left_iterator(map_left.lower_bound(left));
  }

  left_iterator upper_bound_left(const Left& left) const {
    return left_iterator(map_left.upper_bound(left));
  }

  right_iterator lower_bound_right(const Right& right) const {
    return right_iterator(map_right.lower_bound(right));
  }

  right_iterator upper_bound_right(const Right& right) const {
    return right_iterator(map_right.upper_bound(right));
  }

  left_iterator begin_left() const {
    return left_iterator(map_left.begin());
  }

  left_iterator end_left() const {
    return left_iterator(map_left.end());
  }

  right_iterator begin_right() const {
    return right_iterator(map_right.begin());
  }

  right_iterator end_right() const {
    return right_iterator(map_right.end());
  }

  bool empty() const {
    return size_ == 0;
  }

  size_type size() const {
    return size_;
  }

  void swap(bimap& other) noexcept {
    map_left.swap(other.map_left);
    map_right.swap(other.map_right);
    std::swap(size_, other.size_);
  }

  friend void swap(bimap& lhs, bimap& rhs) noexcept {
    lhs.swap(rhs);
  }

  friend bool operator==(const bimap& a, const bimap& b) {
    if (a.size_ != b.size_) {
      return false;
    }

    for (auto it1 = a.begin_left(), it2 = b.begin_left(); it1 != a.end_left(); ++it1, ++it2) {
      if (!a.map_left.equals(*it1, *it2) || !a.map_right.equals(*it1.flip(), *it2.flip())) {
        return false;
      }
    }

    return true;
  }

  friend bool operator!=(const bimap& a, const bimap& b) {
    return !(a == b);
  }
};
