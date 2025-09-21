#include "bitset.hpp"
#include <cstring> 

BitSet::BitSet(size_t nbits)
    : nbits_(nbits),
      nwords_((nbits + 63) / 64),
      data_(new uint64_t[nwords_]()) {}

BitSet::BitSet(const BitSet& other)
    : nbits_(other.nbits_),
      nwords_(other.nwords_),
      data_(new uint64_t[other.nwords_]) {
    std::memcpy(data_, other.data_, nwords_ * sizeof(uint64_t));
}

BitSet& BitSet::operator=(const BitSet& other) {
    if (this != &other) {
        delete[] data_;
        nbits_ = other.nbits_;
        nwords_ = other.nwords_;
        data_ = new uint64_t[nwords_];
        std::memcpy(data_, other.data_, nwords_ * sizeof(uint64_t));
    }
    return *this;
}

BitSet::~BitSet() {
    delete[] data_;
}

void BitSet::set(size_t pos, bool value) {
    check(pos);
    if (value)
        data_[pos >> 6] |= (uint64_t(1) << (pos & 63));
    else
        data_[pos >> 6] &= ~(uint64_t(1) << (pos & 63));
}

bool BitSet::at(size_t pos) const {
    check(pos);
    return (data_[pos >> 6] >> (pos & 63)) & 1;
}

size_t BitSet::size() const {
    return nbits_;
}

void BitSet::check(size_t pos) const {
    if (pos >= nbits_) throw std::out_of_range("BitSet index out of range");
}
