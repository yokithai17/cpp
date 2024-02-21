#include "/home/yoki/cpp/include/string.hpp"

namespace my {
String::String():
    capasity_(1ULL)
    , size_(1ULL)
    , buffer_(nullptr)
{}

String::String(size_t size, char chr):
    capasity_(size)
    , size_(size)
    , buffer_(new char[size])
{
    memset(buffer_, chr, size);
}

String::String(const char str[]):
    capasity_(strlen(str))
    , size_(capasity_)
    , buffer_(new char[capasity_])
{
    memcpy(buffer_, str, size_);
}

String::String(const String& str): String(str.size_, '\0') {
    memcpy(buffer_, str.buffer_, size_);
}


bool operator==(const String& left, const String& right) {
    if (left.size_ != right.size_) {
        return false;
    }
    
    for (size_t i = 0; i < left.size_; ++i) {
        if (left.buffer_[i] != right.buffer_[i]) {
            return false;
        }
    }

    return true;
}

const char& String::operator[](size_t index) const {
    if (index >= size_) { throw; }

    return buffer_[index];
}

char& String::operator[](size_t index) {
    return const_cast<char &>(static_cast<const String&>(*this)[index]);
}

bool operator==(const String& left, const String& right) {
    if (left.size_ != right.size_) {
        return false;
    }

    for (size_t i = 0; i < left.size_; ++i) {
        if (left.buffer_[i] != right.buffer_[i]) {
            return false;
        }
    }

    return true;
}

size_t String::length() const {
    return this->size_;
}

void String::push_back(char chr) {
    if (capasity_ == size_) {
        increase_buff();
    }

    buffer_[size_++] = chr;
}

void String::pop_back() {
    if (size_ == 0) {
        throw;
    }
    if (size_ * 4 == capasity_) {
        decrease_buff();
    }
}

String::~String() {
    delete[] buffer_;
}

void String::increase_buff() {
    capasity_ *= 2;
    char* tmp = new char[capasity_];
    memcpy(tmp, buffer_, size_); 
}

}