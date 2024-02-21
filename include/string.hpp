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

  friend bool operator==(const String&, const String&);

  char& operator[](size_t);

  const char& operator[](size_t) const;

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

  friend std::ostream& operator<<(std::ostream& out, const String&);
  
  friend std::ifstream& operator>>(std::ifstream& in, const String&);
  
  ~String();

 private:
  void increase_buff();

  void decrease_buff();

  size_t capasity_;
  size_t size_;
  char* buffer_;
};
}