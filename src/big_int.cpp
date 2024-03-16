#include "../include/big_int.hpp"

//=================================
// big int constructors
bigint::bigint(): value_('0'), sign_('+') {}

bigint::bigint(const bigint& num): value_(num.value_), sign_(num.sign_) {}

bigint::bigint(const long long& num): value_(std::to_string(std::abs(num))), sign_(((num < 0)?('-'):('+'))) {}

bigint::bigint(const std::string& num) {
    if (num[0] == '+' || num[0] == '-') {
        std::string number = num.substr(1);
        
        if (is_valid_number(number)) {
            this->value_ = number;
            this->sign_ = num[0];
        } else {
            throw std::invalid_argument("Expected an number, got \'" + num + "\'");
        }
    } else {
        if (is_valid_number(num)) {
            this->value_ = num;
            this->sign_ = '+';
        } else {
            throw std::invalid_argument("Expected an number, got \'" + num + "\'");
        }
    }

    strip_leading_zeroes(this->value_);
}