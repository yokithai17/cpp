#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace intrusive {

class default_tag;

class list_element_base {
public:
  list_element_base() noexcept
      : prev_(this)
      , next_(this) {}

  list_element_base(const list_element_base&)
      : list_element_base() {}

  list_element_base(list_element_base&& other) noexcept
      : list_element_base() {
    *this = std::move(other);
  }

  ~list_element_base() {
    unlink();
  }

  list_element_base& operator=(const list_element_base& other) noexcept {
    if (this != &other) {
      unlink();
    }
    return *this;
  }

  list_element_base& operator=(list_element_base&& other) noexcept {
    if (this != &other) {
      unlink();
      if (other.is_linked()) {
        prev_ = std::exchange(other.prev_, &other);
        next_ = std::exchange(other.next_, &other);
        prev_->next_ = this;
        next_->prev_ = this;
      }
    }

    return *this;
  }

  bool is_linked() const noexcept {
    return prev_ != this;
  }

  void unlink() noexcept {
    prev_->next_ = next_;
    next_->prev_ = prev_;
    prev_ = next_ = this;
  }

  void link_before(list_element_base* node) noexcept {
    if (this != node) {
      node->unlink();
      node->prev_ = prev_;
      node->next_ = this;
      prev_->next_ = node;
      prev_ = node;
    }
  }

  void transfer(list_element_base* first, list_element_base* last) noexcept {
    list_element_base* prev_last = last->prev_;
    first->prev_->next_ = last;
    last->prev_ = first->prev_;

    first->prev_ = this->prev_;
    this->prev_->next_ = first;

    this->prev_ = prev_last;
    prev_last->next_ = this;
  }

  list_element_base* prev_;
  list_element_base* next_;
};

template <typename Tag = default_tag>
class list_element : list_element_base {
  template <typename T, typename Tag_>
  friend class list;
};

template <typename T, typename Tag = default_tag>
class list {
  using node_type = list_element<Tag>;
  using node_base = typename node_type::list_element_base;

  static_assert(std::is_base_of_v<list_element<Tag>, T>, "T must derive from list_element");

  template <bool IsConst>
  class based_iterator {
    friend class list;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;

    based_iterator() = default;

    explicit based_iterator(node_base* node_t) noexcept
        : current_(node_t) {}

    explicit based_iterator(const node_base* node_t) noexcept
        : current_(const_cast<node_base*>(node_t)) {}

    template <bool IsConst_>
    based_iterator(const based_iterator<IsConst_>& other) noexcept
        : current_(other.current_) {}

    based_iterator& operator++() {
      current_ = current_->next_;
      return *this;
    }

    based_iterator& operator--() {
      current_ = current_->prev_;
      return *this;
    }

    based_iterator operator++(int) {
      based_iterator copy = *this;
      current_ = current_->next_;
      return copy;
    }

    based_iterator operator--(int) {
      based_iterator copy = *this;
      current_ = current_->prev_;
      return copy;
    }

    template <bool IsConst_>
    bool operator==(const based_iterator<IsConst_>& rhs) const {
      return current_ == rhs.current_;
    }

    template <bool IsConst_>
    bool operator!=(const based_iterator<IsConst_>& rhs) const {
      return current_ != rhs.current_;
    }

    reference operator*() const {
      return as_value(current_);
    }

    pointer operator->() const {
      auto tmp = static_cast<node_type*>(current_);
      return static_cast<pointer>(tmp);
    }

  private:
    node_base* current_;
  };

public:
  using iterator = based_iterator<false>;
  using const_iterator = based_iterator<true>;

  // O(1)
  list() noexcept = default;

  // O(1)
  ~list() = default;

  list(const list&) = delete;
  list& operator=(const list&) = delete;

  // O(1)
  list(list&& other) noexcept = default;

  // O(1)
  list& operator=(list&& other) noexcept = default;

  // O(1)
  bool empty() const noexcept {
    return !dummy_head_.is_linked();
  }

  // O(n)
  size_t size() const noexcept {
    return std::distance(begin(), end());
  }

  // O(1)
  T& front() noexcept {
    return *begin();
  }

  // O(1)
  const T& front() const noexcept {
    return *begin();
  }

  // O(1)
  T& back() noexcept {
    return *std::prev(end());
  }

  // O(1)
  const T& back() const noexcept {
    return *std::prev(end());
  }

  // O(1)
  void push_front(T& value) noexcept {
    dummy_head_.next_->link_before(as_node(value));
  }

  // O(1)
  void push_back(T& value) noexcept {
    dummy_head_.link_before(as_node(value));
  }

  // O(1)
  void pop_front() noexcept {
    if (!empty()) {
      dummy_head_.next_->unlink();
    }
  }

  // O(1)
  void pop_back() noexcept {
    if (!empty()) {
      dummy_head_.prev_->unlink();
    }
  }

  // O(1)
  void clear() noexcept {
    dummy_head_.unlink();
  }

  // O(1)
  iterator begin() noexcept {
    return iterator(dummy_head_.next_);
  }

  // O(1)
  const_iterator begin() const noexcept {
    return const_iterator(dummy_head_.next_);
  }

  // O(1)
  iterator end() noexcept {
    return iterator(&dummy_head_);
  }

  // O(1)
  const_iterator end() const noexcept {
    return const_iterator(&dummy_head_);
  }

  // O(1)
  iterator insert(const_iterator pos, T& value) noexcept {
    pos.current_->link_before(as_node(value));
    return iterator(pos.current_->prev_);
  }

  // O(1)
  iterator erase(const_iterator pos) noexcept {
    if (pos == end()) {
      return pos;
    }

    auto prev = pos.current_->next_;
    pos.current_->unlink();
    return iterator(prev);
  }

  // O(1)
  void splice(const_iterator pos, list&, const_iterator first, const_iterator last) noexcept {
    if (first == last) {
      return;
    }

    pos.current_->transfer(first.current_, last.current_);
  }

private:
  static node_base* as_node(T& value) noexcept {
    auto tmp = static_cast<node_type*>(&value);
    return static_cast<node_base*>(tmp);
  }

  static T& as_value(node_base* node) noexcept {
    auto tmp = static_cast<node_type*>(node);
    return static_cast<T&>(*tmp);
  }

  node_base dummy_head_;
};

} // namespace intrusive
