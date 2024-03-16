#ifndef __BIG_INT_HPP__
#define __BIG_INT_HPP__

#include <iostream>
#include <string>

class bigint {
private:
  std::string value_;
  char sign_;

public:
  bigint();
  bigint(const bigint &);
  bigint(const long long &);
  bigint(const std::string &);

  bigint& operator=(const bigint &);
  bigint& operator=(const long long &);
  bigint& operator=(const std::string &);

  bigint operator+() const; // unary +
  bigint operator-() const; // unary -

  // Binary arithmetic operators:
  bigint operator+(const bigint &) const;
  bigint operator-(const bigint &) const;
  bigint operator*(const bigint &) const;
  bigint operator/(const bigint &) const;
  bigint operator%(const bigint &) const;
  bigint operator+(const long long &) const;
  bigint operator-(const long long &) const;
  bigint operator*(const long long &) const;
  bigint operator/(const long long &) const;
  bigint operator%(const long long &) const;
  bigint operator+(const std::string &) const;
  bigint operator-(const std::string &) const;
  bigint operator*(const std::string &) const;
  bigint operator/(const std::string &) const;
  bigint operator%(const std::string &) const;

  // Arithmetic-assignment operators:
  bigint &operator+=(const bigint &);
  bigint &operator-=(const bigint &);
  bigint &operator*=(const bigint &);
  bigint &operator/=(const bigint &);
  bigint &operator%=(const bigint &);
  bigint &operator+=(const long long &);
  bigint &operator-=(const long long &);
  bigint &operator*=(const long long &);
  bigint &operator/=(const long long &);
  bigint &operator%=(const long long &);
  bigint &operator+=(const std::string &);
  bigint &operator-=(const std::string &);
  bigint &operator*=(const std::string &);
  bigint &operator/=(const std::string &);
  bigint &operator%=(const std::string &);

  // Increment and decrement operators:
  bigint &operator++();   // pre-increment
  bigint &operator--();   // pre-decrement
  bigint operator++(int); // post-increment
  bigint operator--(int); // post-decrement

  // Relational operators:
  bool operator<(const bigint &) const;
  bool operator>(const bigint &) const;
  bool operator<=(const bigint &) const;
  bool operator>=(const bigint &) const;
  bool operator==(const bigint &) const;
  bool operator!=(const bigint &) const;
  bool operator<(const long long &) const;
  bool operator>(const long long &) const;
  bool operator<=(const long long &) const;
  bool operator>=(const long long &) const;
  bool operator==(const long long &) const;
  bool operator!=(const long long &) const;
  bool operator<(const std::string &) const;
  bool operator>(const std::string &) const;
  bool operator<=(const std::string &) const;
  bool operator>=(const std::string &) const;
  bool operator==(const std::string &) const;
  bool operator!=(const std::string &) const;

  // I/O stream operators:
  friend std::istream &operator>>(std::istream &, bigint &);
  friend std::ostream &operator<<(std::ostream &, const bigint &);

  // Conversion functions:
  std::string to_string() const;
  int to_int() const;
  long to_long() const;
  long long to_long_long() const;

};

#endif

#ifndef BIG_INT_HELPERS_HPP
#define BIG_INT_HELPERS_HPP
#include <tuple>

bool is_valid_number(const std::string&);
void strip_leading_zeroes(std::string&);

void add_leading_zeroes(std::string&, size_t);
void add_trailing_zeroes(std::string&, size_t);

std::tuple<std::string, std::string>
get_larger_and_smaller(const::string&,const std::string&);


bool is_power_of_10(const std::string&);
#endif // __BIG_INT_HPP__