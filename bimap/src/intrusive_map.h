#pragma once

#include "iterator.h"

namespace intrusive {
template <bool IsLeft, typename T, typename Compare>
struct intrusive_map {
  using value_type = T;
  using reference = const T&;
  using difference_type = std::ptrdiff_t;

  using node_t = map_element<IsLeft, T>;

  /******************** algosi ********************************************/

  void zig(map_element_base* child) const {
    map_element_base* parent = child->parent;
    if (parent->left == child) {
      map_element_base* child_right = child->right;
      child->right = parent;
      parent->parent = child;
      parent->left = child_right;
      if (child_right) {
        child_right->parent = parent;
      }
    } else {
      map_element_base* child_left = child->left;
      child->left = parent;
      parent->parent = child;
      parent->right = child_left;
      if (child_left) {
        child_left->parent = parent;
      }
    }
    child->parent = nullptr;
  }

  void zig_zig(map_element_base* child) const {
    map_element_base* parent = child->parent;
    map_element_base* grand_parent = parent->parent;
    if (grand_parent->left == parent && parent->left == child) {
      map_element_base* child_right = child->right;
      map_element_base* parent_child = parent->right;
      child->parent = grand_parent->parent;
      if (child->parent) {
        if (child->parent->right == grand_parent) {
          child->parent->right = child;
        } else {
          child->parent->left = child;
        }
      }
      grand_parent->parent = parent;
      parent->right = grand_parent;
      parent->parent = child;
      child->right = parent;
      parent->left = child_right;
      grand_parent->left = parent_child;
      if (child_right) {
        child_right->parent = parent;
      }
      if (parent_child) {
        parent_child->parent = grand_parent;
      }
    } else {
      map_element_base* child_parent = parent->left;
      map_element_base* child_left = child->left;
      child->parent = grand_parent->parent;
      if (child->parent) {
        if (child->parent->right == grand_parent) {
          child->parent->right = child;
        } else {
          child->parent->left = child;
        }
      }
      grand_parent->parent = parent;
      parent->left = grand_parent;
      parent->parent = child;
      child->left = parent;
      parent->right = child_left;
      grand_parent->right = child_parent;
      if (child_parent) {
        child_parent->parent = grand_parent;
      }
      if (child_left) {
        child_left->parent = parent;
      }
    }
  }

  void zig_zag(map_element_base* child) const {
    map_element_base* parent = child->parent;
    map_element_base* grand_parent = parent->parent;
    if (grand_parent->left == parent && parent->right == child) {
      map_element_base* child_left = child->left;
      map_element_base* child_right = child->right;
      child->parent = grand_parent->parent;
      if (child->parent) {
        if (child->parent->right == grand_parent) {
          child->parent->right = child;
        } else {
          child->parent->left = child;
        }
      }
      child->right = grand_parent;
      child->left = parent;
      grand_parent->parent = child;
      parent->parent = child;
      parent->right = child_left;
      grand_parent->left = child_right;
      if (child_left) {
        child_left->parent = parent;
      }
      if (child_right) {
        child_right->parent = grand_parent;
      }
    } else {
      map_element_base* child_left = child->left;
      map_element_base* child_right = child->right;
      child->parent = grand_parent->parent;
      if (child->parent) {
        if (child->parent->right == grand_parent) {
          child->parent->right = child;
        } else {
          child->parent->left = child;
        }
      }
      child->left = grand_parent;
      child->right = parent;
      grand_parent->parent = child;
      parent->parent = child;
      grand_parent->right = child_left;
      parent->left = child_right;
      if (child_left) {
        child_left->parent = grand_parent;
      }
      if (child_right) {
        child_right->parent = parent;
      }
    }
  }

  map_element_base* splay_(map_element_base* child) const {
    while (child->parent) {
      map_element_base* parent = child->parent;
      map_element_base* grand_parent = parent->parent;
      if (!grand_parent) {
        zig(child);
      } else if ((grand_parent->left == parent && parent->left == child) ||
                 (grand_parent->right == parent && parent->right == child)) {
        zig_zig(child);
      } else {
        zig_zag(child);
      }
    }
    return child;
  }

  map_element_base* merge(map_element_base* left, map_element_base* right) const {
    if (!left && !right) {
      return nullptr;
    }
    if (left && !right) {
      left->parent = nullptr;
      return left;
    }
    if (!left) {
      right->parent = nullptr;
      return right;
    }
    left->parent = nullptr;
    left = splay_(map_element_base::tree_max(left));
    left->right = right;
    right->parent = left;
    return left;
  }

  void splay(map_element_base* node) const {
    fake.left->parent = nullptr;
    fake.left = splay_(node);
    if (fake.left) {
      fake.left->parent = &fake;
    }
  }

  /****************************************************************/

  bool empty() const noexcept {
    return fake.left == nullptr;
  }

  void connect_map(map_element_base* other) const noexcept {
    fake.right = other;
  }

  explicit intrusive_map(Compare cmp = Compare()) noexcept
      : comp(std::move(cmp)) {}

  intrusive_map(intrusive_map&& other) noexcept
      : comp(std::move(other.comp))
      , fake(std::move(other.fake)) {}

  intrusive_map& operator=(intrusive_map&& other) noexcept {
    if (this != &other) {
      intrusive_map(std::move(other)).swap(*this);
    }
    return *this;
  }

  map_element_base* insert(map_element_base* n) {
    map_element_base* prev = find_(get_key_(n), false);
    n->parent = prev;
    if (prev == &fake) {
      fake.left = n;
    } else if (comp(get_key_(prev), get_key_(n))) {
      prev->right = n;
    } else {
      prev->left = n;
    }
    splay(n);
    return n;
  }

  void erase(map_element_base* x) noexcept {
    splay(x);
    fake.left = merge(x->left, x->right);
    x->left = nullptr;
    x->right = nullptr;
    if (fake.left) {
      fake.left->parent = &fake;
    }
  }

  map_element_base* find(reference target) const {
    auto* t = find_(target, true);
    return t ? t : end();
  }

  map_element_base* find_(reference target, const bool x_bound) const {
    map_element_base* prev = &fake;
    map_element_base* curr = fake.left;
    while (curr) {
      prev = curr;
      auto& curr_key = get_key_(curr);
      if (comp(target, curr_key)) {
        curr = curr->left;
      } else if (comp(curr_key, target)) {
        curr = curr->right;
      } else {
        return curr;
      }
    }
    return x_bound ? nullptr : prev;
  }

  map_element_base* lower_bound(reference key) const {
    map_element_base* found = find_(key, false);
    if (found == end() || (equals(key, get_key_(found)) || comp(key, get_key_(found)))) {
      return found;
    }
    return map_element_base::next(found);
  }

  map_element_base* upper_bound(reference key) const {
    map_element_base* found = find_(key, false);
    if (found == end() || comp(key, get_key_(found))) {
      return found;
    }
    return map_element_base::next(found);
  }

  map_element_base* begin() const noexcept {
    if (fake.left == nullptr) {
      return end();
    }
    return map_element_base::tree_min(fake.left);
  }

  map_element_base* end() const noexcept {
    return &fake;
  }

  bool equals(reference key1, reference key2) const {
    return !comp(key1, key2) && !comp(key2, key1);
  }

  void swap(intrusive_map& other) noexcept {
    std::swap(fake, other.fake);
    if (fake.left) {
      fake.left->parent = &fake;
    }
    if (other.fake.left) {
      other.fake.left->parent = &other.fake;
    }

    std::swap(comp, other.comp);
  }

  reference get_key_(map_element_base* const& x) const noexcept {
    return static_cast<node_t*>(x)->get();
  }

  [[no_unique_address]] Compare comp;
  mutable map_element_base fake;
};
} // namespace intrusive
