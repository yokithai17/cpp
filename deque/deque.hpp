#include <type_traits>
#include <iostream>

template<typename T>
class DequeIterator;


/// ################################################################################
/// #############################  Dequeue Declaration #############################
/// ################################################################################


template<typename T>
struct Deque {
public:
  Deque();

  Deque(const Deque& other);

  explicit Deque(int quantity);

  Deque(int quantity, const T& element);

  Deque& operator=(const Deque& other);

  size_t size() const;

  void push_back(const T& value);

  void push_front(const T& value);

  void pop_back();

  void pop_front();

  T& operator[](size_t pos);

  const T& operator[](size_t pos) const;

  T& at(size_t pos);

  const T& at(size_t pos) const;

  DequeIterator<T> begin();

  DequeIterator<T> end();

  DequeIterator<const T> begin() const;

  DequeIterator<const T> end() const;

  DequeIterator<const T> cbegin() const;

  DequeIterator<const T> cend() const;

  DequeIterator<T> rbegin();

  DequeIterator<T> rend();

  DequeIterator<const T> rbegin() const;

  DequeIterator<const T> rend() const;

  DequeIterator<const T> crbegin() const;

  DequeIterator<const T> crend() const;

  void insert(const DequeIterator<T>& it, const T& elem);

  void erase(const DequeIterator<T>& it);

  ~Deque();

  using iterator = DequeIterator<T>;
  using const_iterator = DequeIterator<const T>;

  friend DequeIterator<T>& DequeIterator<T>::operator++();

  friend DequeIterator<T>& DequeIterator<T>::operator--();

  friend void DequeIterator<T>::Increase();

  friend void DequeIterator<T>::Decrease();

  friend class DequeIterator<T>;

  friend class DequeIterator<const T>;

private:
  void Expand();

  void Delete();

  static const size_t kInnerSize = 8;
  static const size_t kDefaultOuterSize = 2;

  size_t outer_size_;
  T** outer_;
  size_t start_;
  size_t finish_;
};


/// ########################################################################################
/// #############################  DequeueIterator Declaration #############################
/// ########################################################################################


template<typename T>
class DequeIterator {
public:
  operator DequeIterator<const T>() {
    return DequeIterator<const T>(position_, outer_pointer_, is_reverse_);
  }

  DequeIterator& operator++();

  DequeIterator& operator--();

  DequeIterator operator++(int);

  DequeIterator operator--(int);

  DequeIterator& operator+=(int value);

  DequeIterator operator-=(int value);

  DequeIterator operator+(int value) const;

  DequeIterator operator-(int value) const;

  int operator-(const DequeIterator& other) const;

  int operator-(const DequeIterator<const typename std::remove_const<T>>& other) const;

  T& operator*();

  const T& operator*() const;

  T* operator->();

  const T* operator->() const;

  friend struct Deque<T>;

  friend struct Deque<typename std::remove_const<T>::type>;

  using value_type = T;
  using difference_type = int;
  using iterator_category = std::random_access_iterator_tag;
  using pointer = value_type*;
  using reference = value_type&;

private:
  void Increase();

  void Decrease();

  int Subtract(const DequeIterator& other) const;

  DequeIterator(size_t pos, T** outer, bool rev);

  size_t position_;
  T** outer_pointer_;
  bool is_reverse_;

  template<typename U>
  friend bool operator<(const DequeIterator<U>& first, const DequeIterator<U>& second);
};


/// ################################################################################
/// #############################  Dequeue Realization #############################
/// ################################################################################


template<typename T>
void Deque<T>::Expand() {
  auto destination = new T*[outer_size_ * 2];
  for (size_t i = 0; i < outer_size_ / 2; ++i) {
    destination[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
  for (size_t i = outer_size_ / 2; i <= outer_size_ / 2 + finish_ / kInnerSize - start_ / kInnerSize; ++i) {
    destination[i] = outer_[i - outer_size_ / 2 + start_ / kInnerSize];
  }
  for (size_t i = outer_size_ / 2 + finish_ / kInnerSize - start_ / kInnerSize + 1; i < outer_size_ * 2; ++i) {
    destination[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
  finish_ = (outer_size_ / 2 + finish_ / kInnerSize - start_ / kInnerSize) * kInnerSize + finish_ % kInnerSize;
  start_ = (outer_size_ / 2) * kInnerSize + start_ % kInnerSize;
  delete[] outer_;
  outer_ = destination;
  outer_size_ *= 2;
}

template<typename T>
void Deque<T>::Delete() {
  if (finish_ >= start_) {
    for (size_t j = 0; j <= finish_ - start_; ++j) {
      (*this)[j].~T();
    }
  }
  for (size_t i = 0; i < outer_size_; ++i) {
    delete[] reinterpret_cast<char*>(outer_[i]);
  }
  delete[] outer_;
}


template<typename T>
Deque<T>::Deque() : outer_size_(kDefaultOuterSize), outer_(new T*[outer_size_]),
                    start_(kInnerSize * kDefaultOuterSize / 2), finish_(start_ - 1) {
  for (size_t i = 0; i < outer_size_; ++i) {
    outer_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
}

template<typename T>
Deque<T>::Deque(const Deque& other) : outer_size_(other.outer_size_), outer_(new T*[outer_size_]),
                                      start_(other.start_), finish_(other.finish_) {
  for (size_t i = 0; i < outer_size_; ++i) {
    outer_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
  size_t pos;
  try {
    for (pos = start_; pos <= finish_; ++pos) {
      new(outer_[pos / kInnerSize] + pos % kInnerSize) T(other[pos - start_]);
    }
  } catch (...) {
    finish_ = pos - 1;
    Delete();
    throw;
  }
}

template<typename T>
Deque<T>::Deque(int quantity) : outer_size_((quantity / kInnerSize + 1) * 2), outer_(new T*[outer_size_]),
                                start_(kInnerSize * kDefaultOuterSize / 2), finish_(start_ + quantity - 1) {
  for (size_t i = 0; i < outer_size_; ++i) {
    outer_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
  size_t pos;
  try {
    for (pos = start_; pos <= finish_; ++pos) {
      new(outer_[pos / kInnerSize] + pos % kInnerSize) T();
    }
  } catch (...) {
    finish_ = pos - 1;
  }
}

template<typename T>
Deque<T>::Deque(int quantity, const T& element) : outer_size_((quantity / kInnerSize + 1) * 2), outer_(new T*[outer_size_]),
                                                  start_(kInnerSize * kDefaultOuterSize / 2), finish_(start_ + quantity - 1) {
  for (size_t i = 0; i < outer_size_; ++i) {
    outer_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
  size_t pos;
  try {
    for (pos = start_; pos <= finish_; ++pos) {
      new(outer_[pos / kInnerSize] + pos % kInnerSize) T(element);
    }
  } catch (...) {
    finish_ = pos - 1;
    Delete();
    throw;
  }
}

template<typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  T** buffer = outer_;
  outer_ = new T*[other.outer_size_];
  for (size_t i = 0; i < other.outer_size_; ++i) {
    outer_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kInnerSize]);
  }
  size_t pos;
  try {
    for (pos = other.start_; pos <= other.finish_; ++pos) {
      new(outer_[pos / kInnerSize] + pos % kInnerSize) T(other[pos - other.start_]);
    }
    for (size_t i = start_; i <= finish_; ++i) {
      buffer[i / kInnerSize][i % kInnerSize].~T();
    }
    for (size_t i = 0; i < outer_size_; ++i) {
      delete[] reinterpret_cast<char*>(buffer[i]);
    }
    delete[] buffer;
    start_ = other.start_;
    finish_ = other.finish_;
    outer_size_ = other.outer_size_;
  } catch (...) {
    size_t saved = finish_;
    finish_ = pos - 1;
    Delete();
    finish_ = saved;
    outer_ = buffer;
    throw;
  }
  return *this;
}

template<typename T>
size_t Deque<T>::size() const { return finish_ - start_ + 1; }

template<typename T>
void Deque<T>::push_back(const T& value) {
  if (finish_ == outer_size_ * kInnerSize - 1) {
    Expand();
  }
  try {
    ++finish_;
    new(outer_[finish_ / kInnerSize] + finish_ % kInnerSize) T(value);
  } catch (...) {
    --finish_;
    throw;
  }
}

template<typename T>
void Deque<T>::push_front(const T& value) {
  if (start_ == 0) {
    Expand();
  }
  try {
    --start_;
    new(outer_[start_ / kInnerSize] + start_ % kInnerSize) T(value);
  } catch (...) {
    ++start_;
    throw;
  }
}

template<typename T>
void Deque<T>::pop_back() {
  outer_[finish_ / kInnerSize][finish_ % kInnerSize].~T();
  --finish_;
}

template<typename T>
void Deque<T>::pop_front() {
  outer_[start_ / kInnerSize][start_ % kInnerSize].~T();
  ++start_;
}

template<typename T>
T& Deque<T>::operator[](size_t pos) {
  pos += start_;
  return outer_[pos / kInnerSize][pos % kInnerSize];
}

template<typename T>
const T& Deque<T>::operator[](size_t pos) const {
  pos += start_;
  return outer_[pos / kInnerSize][pos % kInnerSize];
}

template<typename T>
T& Deque<T>::at(size_t pos) {
  if (pos < 0 or pos >= size()) {
    throw std::out_of_range("Deque index out of range");
  }
  pos += start_;
  return outer_[pos / kInnerSize][pos % kInnerSize];
}

template<typename T>
const T& Deque<T>::at(size_t pos) const {
  if (pos < start_ or pos >= finish_) {
    throw std::out_of_range("Deque index out of range");
  }
  pos += start_;
  return outer_[pos / kInnerSize][pos % kInnerSize];
}

template<typename T>
DequeIterator<T> Deque<T>::begin() { return DequeIterator<T>(start_, outer_ + (start_ / kInnerSize), false); }

template<typename T>
DequeIterator<T> Deque<T>::end() { return DequeIterator<T>((finish_ + 1), outer_ + ((finish_ + 1) / kInnerSize), false); }

template<typename T>
DequeIterator<const T> Deque<T>::begin() const {
  return cbegin();
}

template<typename T>
DequeIterator<const T> Deque<T>::end() const {
  return cend();
}

template<typename T>
DequeIterator<const T> Deque<T>::cbegin() const {
  return DequeIterator<const T>(start_, const_cast<const T**>(outer_ + (start_ / kInnerSize)), false);
}

template<typename T>
DequeIterator<const T> Deque<T>::cend() const {
  return DequeIterator<const T>((finish_ + 1), const_cast<const T**>(outer_ + ((finish_ + 1) / kInnerSize)), false);
}

template<typename T>
DequeIterator<T> Deque<T>::rbegin() { return DequeIterator<T>(finish_, outer_ + (finish_ / kInnerSize), true); }

template<typename T>
DequeIterator<T> Deque<T>::rend() { return DequeIterator<T>((start_ - 1), outer_ + ((start_ - 1) / kInnerSize), true); }

template<typename T>
DequeIterator<const T> Deque<T>::rbegin() const {
  return crbegin();
}

template<typename T>
DequeIterator<const T> Deque<T>::rend() const {
  return crend();
}

template<typename T>
DequeIterator<const T> Deque<T>::crbegin() const {
  return DequeIterator<const T>(finish_, const_cast<const T**>(outer_ + (finish_ / kInnerSize), true));
}

template<typename T>
DequeIterator<const T> Deque<T>::crend() const {
  return DequeIterator<const T>((start_ - 1), const_cast<const T**>(outer_ + ((start_ - 1) / kInnerSize)), true);
}

template<typename T>
void Deque<T>::insert(const DequeIterator<T>& it, const T& elem) {
  if (it == end()) {
    push_back(elem);
    return;
  }
  auto x = *rbegin();
  push_back(x);
  DequeIterator forward = --(--end());
  DequeIterator backward = --end();
  while (backward != it) {
    *backward = *forward;
    --backward;
    --forward;
  }
  *backward = elem;
}

template<typename T>
void Deque<T>::erase(const DequeIterator<T>& it) {
  DequeIterator forward = DequeIterator(it.position_, it.outer_pointer_, false);
  forward.is_reverse_ = false;
  DequeIterator backward = forward;
  ++forward;
  DequeIterator stop = end();
  while (forward != stop) {
    *backward = *forward;
    ++backward;
    ++forward;
  }
  pop_back();
}

template<typename T>
Deque<T>::~Deque() { Delete(); }


/// ########################################################################################
/// #############################  DequeueIterator Realization #############################
/// ########################################################################################


template<typename T>
void DequeIterator<T>::Increase() {
  if (position_ % Deque<T>::kInnerSize == Deque<T>::kInnerSize - 1) {
    ++outer_pointer_;
  }
  ++position_;
}

template<typename T>
void DequeIterator<T>::Decrease() {
  if (position_ % Deque<T>::kInnerSize == 0) {
    --outer_pointer_;
  }
  --position_;
}


template<typename T>
int DequeIterator<T>::Subtract(const DequeIterator& other) const {
  int buff = position_ - other.position_;
  if (is_reverse_) {
    buff *= -1;
  }
  return buff;
}

template<typename T>
DequeIterator<T>::DequeIterator(size_t pos, T** outer, bool rev) : position_(pos), outer_pointer_(outer), is_reverse_(rev) {}

template<typename T>
DequeIterator<T>& DequeIterator<T>::operator++() {
  if (!is_reverse_) {
    Increase();
  } else {
    Decrease();
  }
  return *this;
}

template<typename T>
DequeIterator<T>& DequeIterator<T>::operator--() {
  if (!is_reverse_) {
    Decrease();
  } else {
    Increase();
  }
  return *this;
}

template<typename T>
DequeIterator<T> DequeIterator<T>::operator++(int) {
  DequeIterator buff(*this);
  ++(*this);
  return buff;
}

template<typename T>
DequeIterator<T> DequeIterator<T>::operator--(int) {
  DequeIterator buff(*this);
  --(*this);
  return buff;
}

template<typename T>
DequeIterator<T>& DequeIterator<T>::operator+=(int value) {
  if (value == 0) {
    return *this;
  }
  if ((value > 0 && !is_reverse_) || (value < 0 && is_reverse_)) {
    if (value < 0) {
      value = -value;
    }
    outer_pointer_ += value / Deque<T>::kInnerSize;
    if (value % Deque<T>::kInnerSize + position_ % Deque<T>::kInnerSize >= Deque<T>::kInnerSize) {
      ++outer_pointer_;
    }
    position_ += value;
  } else {
    if (value < 0) {
      value = -value;
    }
    outer_pointer_ -= value / Deque<T>::kInnerSize;
    if (position_ % Deque<T>::kInnerSize - value % Deque<T>::kInnerSize >= Deque<T>::kInnerSize) {
      --outer_pointer_;
    }
    position_ -= value;
  }
  return *this;
}

template<typename T>
DequeIterator<T> DequeIterator<T>::operator-=(int value) { return *this += -value; }

template<typename T>
DequeIterator<T> DequeIterator<T>::operator+(int value) const {
  DequeIterator buff(*this);
  buff += value;
  return buff;
}

template<typename T>
DequeIterator<T> DequeIterator<T>::operator-(int value) const { return *this + -value; }

template<typename T>
int DequeIterator<T>::operator-(const DequeIterator& other) const {
  return Subtract(other);
}

template<typename T>
int DequeIterator<T>::operator-(const DequeIterator<const typename std::remove_const<T>>& other) const {
  return Subtract(other);
}

template<typename T>
bool operator<(const DequeIterator<T>& first, const DequeIterator<T>& second) { return (first.position_ != second.position_) && (first.position_ < second.position_) ^ (first.is_reverse_); }

template<typename T>
bool operator==(const DequeIterator<T>& first, const DequeIterator<T>& second) { return !(first < second) && !(second < first); }

template<typename T>
bool operator!=(const DequeIterator<T>& first, const DequeIterator<T>& second) { return !(first == second); }

template<typename T>
bool operator>=(const DequeIterator<T>& first, const DequeIterator<T>& second) { return !(first < second); }

template<typename T>
bool operator>(const DequeIterator<T>& first, const DequeIterator<T>& second) { return (first != second && first >= second); }

template<typename T>
bool operator<=(const DequeIterator<T>& first, const DequeIterator<T>& second) { return (first == second || first < second); }

template<typename T>
T& DequeIterator<T>::operator*() { return (*outer_pointer_)[position_ % Deque<T>::kInnerSize]; }

template<typename T>
const T& DequeIterator<T>::operator*() const { return (*outer_pointer_)[position_ % Deque<T>::kInnerSize]; }

template<typename T>
T* DequeIterator<T>::operator->() { return &(*outer_pointer_)[position_ % Deque<T>::kInnerSize]; }

template<typename T>
const T* DequeIterator<T>::operator->() const { return &(*outer_pointer_)[position_ % Deque<T>::kInnerSize]; }
