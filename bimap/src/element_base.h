#include <utility>

namespace intrusive {
struct map_element_base {
  map_element_base* left{nullptr};
  map_element_base* right{nullptr};
  map_element_base* parent{nullptr};

  map_element_base() = default;

  ~map_element_base() noexcept {
    reset();
  }

  map_element_base(map_element_base* left, map_element_base* right, map_element_base* parent) noexcept
      : left(left)
      , right(right)
      , parent(parent) {}

  map_element_base(map_element_base&& other) {
    swap(other);
  }

  map_element_base& operator=(map_element_base&& other) noexcept {
    if (this != &other) {
      map_element_base(std::move(other)).swap(*this);
    }
    return *this;
  }

  bool is_left() {
    return parent->left == this;
  }

  void swap(map_element_base& other) noexcept {
    relink_parent(&other);
    other.relink_parent(this);
    std::swap(left, other.left);
    std::swap(right, other.right);
    std::swap(parent, other.parent);
    relink_parents();
    other.relink_parents();
  }

  void relink_parent(map_element_base* other) noexcept {
    if (parent) {
      if (parent->left == this) {
        parent->left = other;
      } else {
        parent->right = other;
      }
    }
  }

  void relink_parents() {
    if (left) {
      left->parent = this;
    }

    if (right) {
      right->parent = this;
    }
  }

  void link_child(map_element_base* child, bool is_left) {
    if (is_left) {
      this->left = child;
    } else {
      this->right = child;
    }
    if (child) {
      child->parent = this;
    }
  }

  void replace(map_element_base* to) {
    to->parent = this->parent;
    to->left = this->left;
    to->right = this->right;

    bool is_left = this->parent->left == this;
    this->parent->link_child(to, is_left);

    if (to->left) {
      to->left->parent = to;
    }
    if (to->right) {
      to->right->parent = to;
    }
  }

  static map_element_base* tree_min(map_element_base* node) {
    if (!node) {
      return node;
    }
    while (node && node->left) {
      node = node->left;
    }
    return node;
  }

  static map_element_base* tree_max(map_element_base* node) {
    if (!node) {
      return node;
    }
    while (node && node->right != nullptr) {
      node = node->right;
    }
    return node;
  }

  static map_element_base* next(map_element_base* node) {
    if (node->right) {
      return tree_min(node->right);
    }
    while (node->parent && node->parent->right == node) {
      node = node->parent;
    }
    return node->parent;
  }

  static map_element_base* prev(map_element_base* node) {
    if (node->left) {
      return tree_max(node->left);
    }
    while (node->parent && node->parent->left == node) {
      node = node->parent;
    }
    return node->parent;
  }

  void reset() {
    left = nullptr;
  }
};

template <bool IsLeft, typename T>
struct map_element : map_element_base {
  map_element() noexcept = default;

  template <typename U = T>
  explicit map_element(U&& storage)
      : value(std::forward<U>(storage)) {}

  const T& get() const {
    return value;
  }

  T& get() {
    return value;
  }

  T value;
};

template <typename Left, typename Right>
struct bimap_element
    : intrusive::map_element<true, Left>
    , intrusive::map_element<false, Right> {
  template <typename L, typename R>
  bimap_element(L&& left, R&& right) try
      : intrusive

    ::map_element<true, Left>(std::forward<L>(left)), intrusive::map_element<false, Right>(std::forward<R>(right)) {}

  catch (...) {
    throw;
  }
};
} // namespace intrusive
