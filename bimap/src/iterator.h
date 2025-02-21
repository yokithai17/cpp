
#include "bimap.h"
#include "element_base.h"

#include <iterator>

namespace intrusive {
template <typename Left, typename Right, bool IsLeft>
struct based_iterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::conditional_t<IsLeft, Left, Right>;
  using pointer = const value_type*;
  using reference = const value_type&;
  using difference_type = std::ptrdiff_t;

  using element_t = bimap_element<Left, Right>;
  using node_t = map_element<IsLeft, value_type>;

  using opposite_iterator = based_iterator<Left, Right, !IsLeft>;
  using opposite_value = std::conditional_t<IsLeft, Right, Left>;
  using opposite_node_t = map_element<!IsLeft, opposite_value>;

  based_iterator() noexcept
      : current() {}

  reference operator*() const {
    return static_cast<node_t*>(current)->get();
  }

  pointer operator->() const {
    return &static_cast<node_t*>(current)->get();
  }

  based_iterator& operator++() {
    current = map_element_base::next(current);
    return *this;
  }

  based_iterator operator++(int) {
    based_iterator copy = *this;
    ++*this;
    return copy;
  }

  based_iterator& operator--() {
    current = map_element_base::prev(current);
    return *this;
  }

  based_iterator operator--(int) {
    based_iterator copy = *this;
    --*this;
    return copy;
  }

  opposite_iterator flip() const {
    if (!current->parent) {
      return opposite_iterator(current->right);
    }
    auto* based = static_cast<element_t*>(static_cast<node_t*>(current));
    return opposite_iterator(static_cast<map_element_base*>(static_cast<opposite_node_t*>(based)));
  }

  bool operator==(const based_iterator& rhs) const noexcept = default;

  bool operator!=(const based_iterator& rhs) const noexcept = default;

  explicit based_iterator(map_element_base* current) noexcept
      : current(current) {}

  map_element_base* get_data() const noexcept {
    return current;
  }

  map_element_base* current;
};
} // namespace intrusive
