#include "../include/string.hpp"

my::String::String()
    : capasity_(1ULL), size_(0ULL), buffer_(new char[capasity_]) {
  buffer_[0] = '\0';
}

my::String::String(size_t size, char chr)
    : capasity_(size + 1), size_(size), buffer_(new char[capasity_]) {
  memset(buffer_, chr, size * sizeof(char));
  buffer_[size_] = '\0';
}

my::String::String(const char str[])
    : capasity_(strlen(str) + 1), size_(capasity_ - 1),
      buffer_(new char[capasity_]) {
  memcpy(buffer_, str, capasity_);
}

my::String::String(const String &str) : my::String(str.size_, '\0') {
  memcpy(buffer_, str.buffer_, size_);
}

const char &my::String::operator[](size_t index) const {
  if (index >= size_) {
    throw;
  }

  return buffer_[index];
}

char &my::String::operator[](size_t index) { return this->buffer_[index]; }

size_t my::String::length() const { return this->size_; }

void my::String::push_back(char chr) {
  buffer_[size_++] = chr;
  if (capasity_ == size_) {
    increase_buff();
  }
}

void my::String::pop_back() {
  if (size_ == 0) {
    throw;
  }
  --size_;
  if (size_ * 4 == capasity_) {
    decrease_buff();
  }
  buffer_[size_] = '\0';
}

const char &my::String::front() const {
  if (size_ == 0) {
    throw;
  }
  return this->buffer_[0];
}

char &my::String::front() {
  return const_cast<char &>(static_cast<const my::String &>(*this).front());
}

const char &my::String::back() const {
  if (size_ == 0) {
    throw;
  }
  return this->buffer_[size_ - 1];
}

char &my::String::back() {
  return const_cast<char &>(static_cast<const my::String &>(*this).back());
}

size_t my::String::find(const my::String &substr) const {
  /*
  struct hitres {
      long long a;
      long long b;
      long long c;
  };

  struct hitres* hitres_t = reinterpret_cast<hitres &>(substr);
  char* chr_p = reinterpret_cast<char *>(hitres_t->a);

  char* it_p = strstr(this->buffer_, chr_p);

  return (this->buffer_ - it_p);
  */
  const char *ptr = strstr(this->buffer_, substr.buffer_);
  if (ptr == NULL) {
    return this->size_;
  }
  return (ptr - this->buffer_);
}

size_t my::String::rfind(const my::String &substr) const {
  size_t it = this->find(substr);
  if (it == this->size_) {
    return it;
  }
  return it + substr.length() - 1;
}

bool my::String::empty() const { return size_ == 0; }

void my::String::clear() {
  delete[] buffer_;
  buffer_ = new char[1];
  buffer_[0] = '\0';
  size_ = 0;
  capasity_ = 1;
}

my::String::~String() { delete[] buffer_; }

void my::String::increase_buff() {
  capasity_ *= 2;
  char *tmp = new char[capasity_];
  memcpy(tmp, buffer_, size_);
  delete[] buffer_;
  buffer_ = tmp;
  buffer_[size_] = '\0';
}

void my::String::decrease_buff() {
  capasity_ /= 2;
  char *tmp = new char[capasity_];
  memcpy(tmp, buffer_, size_);
  delete[] buffer_;
  buffer_ = tmp;
  buffer_[size_] = '\0';
}
