#include <iostream>
#include <vector>
#include <cassert>

enum class Sign : int {
  Negative = -1,
  Neutral = 0,
  Positive = 1
};

Sign operator-(const Sign sign) {
  return static_cast<Sign>(-static_cast<int>(sign));
}

class BigInteger {
private:
  static const int kTo_string_base_ = 10;
  static const int kBase_ = 1e9;
  static const int kExp_ = 9;
  std::vector<int> bits_;
  Sign sign_;

  static void CutZeros(std::vector<int>& input) {
    if (input.back() != 0) {
      return;
    }
    int pin = input.size() - 1;
    while (pin > 0 && input[pin] == 0) {
      input.pop_back();
      --pin;
    }
  }

  void Fit() {
    for (int pos = bits_.size() - 1; pos > 0; --pos) {
      if (bits_[pos] == 0) {
        bits_.pop_back();
      } else {
        break;
      }
    }
    if (bits_.size() == 1 && bits_[0] == 0) {
      sign_ = Sign::Neutral;
    }
  }

  static void Add(std::vector<int>& first, const std::vector<int>& second) {
    std::vector<int> neutral = {0};
    if (first == neutral) {
      first = second;
      return;
    }
    if (second == neutral) {
      return;
    }
    while (first.size() < second.size()) {
      first.push_back(0);
    }
    first.push_back(0);
    int buf = 0;
    for (size_t i = 0; i < second.size(); ++i) {
      first[i] += second[i] + buf;
      if (first[i] >= kBase_) {
        first[i] -= kBase_;
        buf = 1;
      } else {
        buf = 0;
      }
    }
    if (buf > 0) {
      first[second.size()] += buf;
    }
  }

  static void Add(const std::vector<int>& first, const std::vector<int>& second, std::vector<int>& destination) {
    destination = first;
    Add(destination, second);
  }

  static void Subtract(std::vector<int>& first, const std::vector<int>& second) {
    const std::vector<int> third = first;
    Subtract(third, second, first);
  }

  static void Subtract(const std::vector<int>& first, const std::vector<int>& second, std::vector<int>& destination) {
    std::vector<int> neutral = {0};
    if (first == neutral) {
      destination = second;
      return;
    }
    if (second == neutral) {
      destination = first;
      return;
    }
    destination = {};
    while (destination.size() < first.size()) {
      destination.push_back(0);
    }
    int buf = 0;
    for (size_t i = 0; i < second.size(); ++i) {
      if (first[i] - buf < second[i]) {
        destination[i] = first[i] + kBase_ - buf - second[i];
        buf = 1;
      } else {
        destination[i] = first[i] - buf - second[i];
        buf = 0;
      }
    }
    for (size_t i = second.size(); i < first.size(); ++i) {
      if (first[i] - buf < 0) {
        destination[i] = first[i] + kBase_ - buf;
        buf = 1;
      } else {
        destination[i] = first[i] - buf;
        buf = 0;
      }
    }
  }

  static void InversedSubtract(std::vector<int>& first, const std::vector<int>& second) {
    std::vector<int> neutral = {0};
    if (first == neutral) {
      first = second;
      return;
    }
    if (second == neutral) {
      return;
    }
    while (first.size() < second.size()) {
      first.push_back(0);
    }
    int buf = 0;
    for (size_t i = 0; i < second.size(); ++i) {
      if (second[i] - buf < first[i]) {
        first[i] = second[i] + kBase_ - buf - first[i];
        buf = 1;
      } else {
        first[i] = second[i] - buf - first[i];
        buf = 0;
      }
    }
    if (buf > 0) {
      first[second.size()] -= buf;
    }
  }

  static void Multiply(const std::vector<int>& first, const std::vector<int>& second, std::vector<int>& destination) {
    std::vector<int> neutral = {1};
    if (first == neutral) {
      destination = second;
      return;
    }
    if (second == neutral) {
      destination = first;
      return;
    }
    std::vector<int> deleter = {0};
    if (first == deleter || second == deleter) {
      destination = deleter;
      return;
    }
    destination = std::vector<int>(first.size() + second.size() + 1, 0);
    std::vector<long long> destiny = std::vector<long long>(first.size() + second.size() + 1, 0);
    for (size_t i = 0; i < second.size(); ++i) {
      long long buf = 0;
      for (size_t j = 0; j < first.size(); ++j) {
        long long nu = 1ll * first[j] * second[i] + buf;
        buf = nu / kBase_;
        destiny[i + j] += nu % kBase_;
      }
      destiny[i + first.size()] += buf;
    }
    long long buf = 0;
    for (size_t i = 0; i < destiny.size(); ++i) {
      int bu = buf;
      buf = (destiny[i] + buf) / kBase_;
      destiny[i] = (destiny[i] + bu) % kBase_;
    }
    for (size_t i = 0; i < destiny.size(); ++i) {
      destination[i] = destiny[i];
    }
  }

  static bool NotHigher(const std::vector<int>& first, const std::vector<int>& second) {
    int delta_1 = 0;
    int delta_2 = 0;
    while (first[first.size() - 1 - delta_1] == 0 && first.size() - 1 - delta_1 > 0) {
      ++delta_1;
    }
    while (second[second.size() - 1 - delta_2] == 0 && second.size() - 1 - delta_2 > 0) {
      ++delta_2;
    }
    if (first.size() - delta_1 < second.size() - delta_2) {
      return true;
    }
    if (first.size() - delta_1 > second.size() - delta_2) {
      return false;
    }
    for (ssize_t i = first.size() - delta_1 - 1; i >= 0; --i) {
      if (first[i] > second[i]) {
        return false;
      }
      if (first[i] < second[i]) {
        return true;
      }
    }
    return true;
  }

  static void Divide(const std::vector<int>& first, const std::vector<int>& second, std::vector<int>& destination) {
    std::vector<int> neutral = {1};
    if (second == neutral) {
      destination = first;
      return;
    }
    std::vector<int> deleter = {0};
    if (first == deleter) {
      destination = deleter;
      return;
    }
    destination = std::vector<int>(first.size() + 1, -1);
    int pin = first.size();
    std::vector<int> buf;
    for (int i = first.size() - 1; i >= 0; --i) {
      buf.insert(buf.begin(), first[i]);
      for (int pos = buf.size() - 1; pos > 0; --pos) {
        if (buf[pos] == 0) {
          buf.pop_back();
        } else {
          break;
        }
      }
      std::vector<int> buf_2 = {0};
      int left = 0;
      int right = kBase_ - 1;
      int mid;
      while (right > left + 1) {
        mid = left + (right - left) / 2;
        buf_2 = {};
        std::vector<int> mi;
        mi.push_back(mid);
        Multiply(second, mi, buf_2);
        if (NotHigher(buf, buf_2)) {
          right = mid;
        } else {
          left = mid;
        }
      }
      buf_2 = {};
      Multiply(std::vector<int>({right}), second, buf_2);
      CutZeros(buf_2);
      int number;
      if (NotHigher(buf_2, buf)) {
        number = right;
      } else {
        Subtract(buf_2, second);
        CutZeros(buf_2);
        number = left;
      }
      destination[pin] = number;
      --pin;
      Subtract(buf, buf_2);
    }
    int start = pin + 1;
    for (size_t i = start; i < destination.size(); ++i) {
      destination[i - start] = destination[i];
    }
    for (size_t i = destination.size() - start; i < destination.size(); ++i) {
      destination.pop_back();
    }
  }

  BigInteger(std::vector<int>& source) {
    bits_ = source;
    sign_ = Sign::Positive;
  }

public:
  BigInteger(int integer) {
    if (integer > 0) {
      sign_ = Sign::Positive;
    } else if (integer < 0) {
      sign_ = Sign::Negative;
    } else {
      sign_ = Sign::Neutral;
    }
    if (integer < 0) {
      integer *= -1;
    }
    if (integer < kBase_) {
      bits_.push_back(integer);
      return;
    }
    if (integer == kBase_) {
      bits_.push_back(1);
      bits_.push_back(0);
      return;
    }
    int max = 0;
    long divider = 1;
    while (divider * kBase_ <= integer) {
      ++max;
      divider *= kBase_;
    }
    divider = kBase_;
    for (; max >= 0; --max) {
      bits_.push_back((integer % divider) / (divider / kBase_));
      divider *= kBase_;
    }
  }

  BigInteger() {
    sign_ = Sign::Neutral;
    bits_.push_back(0);
  };

  BigInteger(std::string str) {
    if (str[0] == '-') {
      sign_ = Sign::Negative;
      str.erase(str.begin());
    } else if (str == "0") {
      sign_ = Sign::Neutral;
      bits_ = {0};
      return;
    } else {
      sign_ = Sign::Positive;
    }
    std::string buf;
    ssize_t i;
    for (i = str.size() - 1; i >= kExp_ - 1; i -= kExp_) {
      buf = "";
      for (int j = kExp_ - 1; j >= 0; j--) {
        buf += str[i - j];
      }
      bits_.push_back(std::atoi(buf.c_str()));
    }
    buf = "";
    for (int j = 0; j <= i; ++j) {
      buf += str[j];
    }
    if (buf.size() > 0) {
      bits_.push_back(std::atoi(buf.c_str()));
    }
  }

  BigInteger(const char* str) {
    std::string st = str;
    *this = BigInteger(st);
  }

  std::string toString() const {
    std::string str;
    for (size_t i = 0; i < bits_.size() ; ++i) {
      int temp = bits_[i];
      for (int j = 0; j < kExp_; ++ j) {
        char ch = temp % kTo_string_base_ + '0';
        temp /= kTo_string_base_;
        str.push_back(ch);
      }
    }
    while (str.back() == '0' && str.size() > 1) {
      str.pop_back();
    }
    if (sign_ == Sign::Negative) {
      str.push_back('-');
    }
    str = {str.rbegin(), str.rend()};
    return str;
  }

  BigInteger operator-() const {
    BigInteger bigInteger = BigInteger(*this);
    bigInteger.sign_ = -sign_;
    return bigInteger;
  }

  friend bool operator==(const BigInteger&, const BigInteger&);

  friend bool operator<(const BigInteger&, const BigInteger&);

  BigInteger& operator+=(const BigInteger& second) {
    if (sign_ == Sign::Neutral) {
      *this = second;
      return *this;
    }
    if (second.sign_ == Sign::Neutral) {
      return *this;
    }
    if (sign_ == second.sign_) {
      Add(bits_, second.bits_);
      Fit();
      return *this;
    }
    if (NotHigher(bits_, second.bits_)) {
      InversedSubtract(bits_, second.bits_);
      sign_ = second.sign_;
      Fit();
      return *this;
    }
    Subtract(bits_, second.bits_);
    Fit();
    return *this;
  }

  BigInteger& operator-=(const BigInteger& second) {
    if (sign_ == Sign::Neutral) {
      *this = -second;
      return *this;
    }
    if (second.sign_ == Sign::Neutral) {
      return *this;
    }
    if (sign_ != second.sign_) {
      Add(bits_, second.bits_);
      Fit();
      return *this;
    }
    if (*this == second) {
      *this = 0;
      return *this;
    }
    if (NotHigher(bits_, second.bits_)) {
      InversedSubtract(bits_, second.bits_);
      sign_ = -second.sign_;
      Fit();
      return *this;
    }
    Subtract(bits_, second.bits_);
    Fit();
    return *this;
  }

  BigInteger& operator*=(const BigInteger& second) {
    if (sign_ == Sign::Neutral) {
      return *this;
    }
    if (second.sign_ == Sign::Neutral) {
      *this = second;
      return *this;
    }
    BigInteger created;
    std::vector<int> source;
    Multiply(bits_, second.bits_, source);
    created = BigInteger(source);
    if (sign_ != second.sign_) {
      created.sign_ = Sign::Negative;
    }
    *this = created;
    Fit();
    return *this;
  }

  BigInteger& operator/=(const BigInteger& second) {
    assert(second.sign_ != Sign::Neutral);
    if (*this == BigInteger(1)) {
      *this = second;
      return *this;
    }
    if (*this == -BigInteger(1)) {
      *this = -second;
      return *this;
    }
    if (*this == BigInteger(0)) {
      *this = BigInteger(0);
      return *this;
    }
    if (second == *this) {
      *this = BigInteger(1);
      return *this;
    }
    if (second == -(*this)) {
      *this = -BigInteger(1);
      return *this;
    }
    if (NotHigher(bits_, second.bits_)) {
      *this = BigInteger(0);
      return *this;
    }
    std::vector<int> source;
    Divide(bits_, second.bits_, source);
    if (sign_ != second.sign_) {
      *this = BigInteger(source);
      sign_ = Sign::Negative;
    } else {
      *this = BigInteger(source);
    }
    Fit();
    return *this;
  }

  BigInteger& operator++() {
    *this += BigInteger(1);
    return *this;
  }

  BigInteger& operator--() {
    *this -= BigInteger(1);
    return *this;
  }

  BigInteger operator++(int) {
    BigInteger bigInteger = *this;
    *this += BigInteger(1);
    return bigInteger;
  }

  BigInteger operator--(int) {
    BigInteger bigInteger = *this;
    *this -= BigInteger(1);
    return bigInteger;
  }

  explicit operator bool() const { return sign_ != Sign::Neutral; }

  explicit operator int() const {
    return atoi(toString().c_str());
  }

  BigInteger& operator%=(const BigInteger&);
};

std::ostream& operator<<(std::ostream& out, const BigInteger& biginteger) {
  return out << biginteger.toString();
}

std::istream& operator>>(std::istream& in, BigInteger& biginteger) {
  std::string str;
  in >> str;
  biginteger = BigInteger(str);
  return in;
}

bool operator==(const BigInteger& first, const BigInteger& second) {
  if (second.bits_.size() != first.bits_.size() || second.sign_ != first.sign_) {
    return false;
  }
  for (size_t i = 0; i < first.bits_.size(); ++i) {
    if (second.bits_[i] != first.bits_[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const BigInteger& first, const BigInteger& second) {
  return !(first == second);
}

bool operator<(const BigInteger& first, const BigInteger& second) {
  if (first.sign_ < second.sign_) {
    return true;
  }
  if (first.sign_ > second.sign_) {
    return false;
  }
  if (first.sign_ == Sign::Neutral) {
    return false;
  }
  bool modifier = first.sign_ > Sign::Neutral;
  if (first.bits_.size() < second.bits_.size()) {
    return modifier;
  }
  if (first.bits_.size() > second.bits_.size()) {
    return !modifier;
  }
  for (ssize_t i = first.bits_.size() - 1; i >= 0; --i) {
    if (first.bits_[i] > second.bits_[i]) {
      return !modifier;
    }
    if (first.bits_[i] < second.bits_[i]) {
      return modifier;
    }
  }
  return false;
}

bool operator>=(const BigInteger& first, const BigInteger& second) {
  return !(first < second);
}

bool operator>(const BigInteger& first, const BigInteger& second) {
  return second < first;
}

bool operator<=(const BigInteger& first, const BigInteger& second) {
  return !(first > second);
}

BigInteger operator"" _bi(const char* number) {
  return BigInteger(number);
}

BigInteger operator+(const BigInteger& first, const BigInteger& second) {
  BigInteger ret = first;
  ret += second;
  return ret;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
  BigInteger ret = first;
  ret -= second;
  return ret;
}

BigInteger operator*(const BigInteger& first, const BigInteger& second) {
  BigInteger ret = first;
  ret *= second;
  return ret;
}

BigInteger operator/(const BigInteger& first, const BigInteger& second) {
  BigInteger ret = first;
  ret /= second;
  return ret;
}

BigInteger& BigInteger::operator%=(const BigInteger& second) {
  assert(second.sign_ != Sign::Neutral);
  *this =  *this - (*this / second) * second;
  Fit();
  return *this;
}

BigInteger operator%(const BigInteger& first, const BigInteger& second) {
  BigInteger ret = first;
  ret %= second;
  return ret;
}

BigInteger GCD(BigInteger first, BigInteger second) {
  if (first < 0) {
    first = -first;
  }
  if (second < 0) {
    second = -second;
  }
  while(first != 0 && second != 0) {
    if (first > second) {
      first %= second;
      continue;
    }
    second %= first;
  }
  return (first == 0) ? second : first;
}

class Rational {
private:
  static const int kDouble_precision_ = 20;
  BigInteger numerator_;
  BigInteger denominator_;

  Rational(const BigInteger& numerator, const BigInteger& denominator){
    BigInteger zero = 0;
    if (denominator < zero) {
      denominator_ = -denominator;
      numerator_ = -numerator;
    } else {
      denominator_ = denominator;
      numerator_ = numerator;
    }
  }

  void Fit() {
    BigInteger core = GCD(numerator_, denominator_);
    numerator_ /= core;
    denominator_ /= core;
  }

public:
  Rational() {
    numerator_ = 0;
    denominator_ = 1;
  }

  Rational(const BigInteger& bigInteger) {
    numerator_ = bigInteger;
    denominator_ = 1;
  }

  Rational(int integer) {
    numerator_ = integer;
    denominator_ = 1;
  }

  Rational operator-() const {
    Rational created = Rational(-numerator_, denominator_);
    return created;
  }

  friend bool operator==(const Rational&, const Rational&);

  friend bool operator<(const Rational&, const Rational&);

  Rational& operator-=(const Rational& second) {
    BigInteger core = GCD(denominator_, second.denominator_);
    *this = Rational(numerator_ * (second.denominator_ / core) - second.numerator_ * (denominator_ / core), denominator_ * second.denominator_ / core);
    Fit();
    return *this;
  }

  Rational& operator+=(const Rational& second) {
    *this -= -second;
    return *this;
  }

  Rational& operator*=(const Rational& second) {
    *this = Rational(numerator_ * second.numerator_, denominator_ * second.denominator_);
    Fit();
    return *this;
  }

  Rational& operator/=(const Rational& second) {
    assert(second.numerator_ != 0);
    *this = Rational(numerator_ * second.denominator_, denominator_ * second.numerator_);
    Fit();
    return *this;
  }

  std::string toString() const {
    std::string str = numerator_.toString();
    if (denominator_ != BigInteger(1)) {
      str += "/" + denominator_.toString();
    }
    return str;
  }

  std::string asDecimal(size_t precision = 0) const {
    assert(precision >= 0);
    std::string str = (numerator_ / denominator_).toString();
    if (str == "0" && numerator_ < 0 && precision > 0) {
      str = "-0";
    }
    if (precision == 0) {
      return str;
    }
    str += ".";
    BigInteger num;
    if (numerator_ < 0) {
      num = -numerator_;
    } else {
      num = numerator_;
    }
    num %= denominator_;
    BigInteger factor = 1;
    for (size_t i = 0; i < precision; ++i) {
      factor *= 10;
    }
    num *= factor;
    std::string small = (num / denominator_).toString();
    if (precision - small.size() > 0) {
      str += std::string(precision - small.size(), '0');
    }
    str += small;
    return str;
  }

  explicit operator double() const {
    std::string str = asDecimal(kDouble_precision_);
    return std::stod(str);
  }
};

bool operator==(const Rational& first, const Rational& second) {
  return first.numerator_ == second.numerator_ && first.denominator_ == second.denominator_;
}

bool operator!=(const Rational& first, const Rational& second) {
  return !(first == second);
}

bool operator<(const Rational& first, const Rational& second) {
  BigInteger core = GCD(first.denominator_, second.denominator_);
  return first.numerator_ * (second.denominator_ / core) < second.numerator_ * (first.denominator_ / core);
}

bool operator>=(const Rational& first, const Rational& second) {
  return !(first < second);
}

bool operator<=(const Rational& first, const Rational& second) {
  return (first < second) || (first == second);
}

bool operator>(const Rational& first, const Rational& second) {
  return !(first < second) && (first != second);
}

Rational operator-(const Rational& first, const Rational& second) {
  Rational created = first;
  created -= second;
  return created;
}

Rational operator+(const Rational& first, const Rational& second) {
  Rational created = first - (-second);
  return created;
}

Rational operator*(const Rational& first, const Rational& second) {
  Rational created = first;
  created *= second;
  return created;
}

Rational operator/(const Rational& first, const Rational& second) {
  Rational created = first;
  created /= second;
  return created;
}

std::ostream& operator<<(std::ostream& out, const Rational& rational) {
  return out << rational.toString();
}

std::istream& operator>>(std::istream& in, Rational& rational) {
  BigInteger input;
  in >> input;
  rational = Rational(input);
  return in;
}