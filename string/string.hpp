#pragma once

#include <iostream>
#include <cstring>

namespace my {
class String {
 public:
  String();

  String(const  char*);

  String(size_t , char);

  String(const String&);

  char& operator[](size_t);

  const char& operator[](size_t) const;

  friend bool operator==(const my::String& left, const my::String& right) {
    if (left.size_ != right.size_) {
      return false;
    }
    return !strcmp(left.buffer_, right.buffer_);
  }

  size_t length() const;

  void push_back(char);
  
  void pop_back();

  const char& front() const;
  
  char& front();

  const char& back() const;

  char& back();

  //String& operator+=(const String&);

  size_t find(const String&) const;
  
  size_t rfind(const String&) const;

  bool empty() const;

  void clear();

  friend std::ostream& operator<<(std::ostream& out, const String& str) {
    out << str.buffer_;
    return out;
  }

  friend std::istream& operator>>(std::istream& in, String& str) {
    char buffer[100];
    in >> buffer;
    str.size_ = strlen(buffer);
    str.capasity_ = str.size_;
    str.buffer_ = new char[str.size_];
    str.buffer_[str.size_] = '\0';
    return in;
  }

  
  ~String();

 private:
  void increase_buff();

  void decrease_buff();

  size_t capasity_;
  size_t size_;
  char* buffer_;
};
}