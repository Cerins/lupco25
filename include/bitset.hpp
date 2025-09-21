#pragma once

#include <cstdint>
#include <cstddef> 
#include <stdexcept>

class BitSet {
public:
    explicit BitSet(size_t nbits);

    // deep copy constructor
    BitSet(const BitSet& other);

    // deep copy assignment
    BitSet& operator=(const BitSet& other);

    // Destructor
    ~BitSet();

    // Set bit at pos to value
    void set(size_t pos, bool value);

    // Get bit at pos
    bool at(size_t pos) const;

    size_t size() const;

private:
    size_t nbits_;
    size_t nwords_;
    uint64_t* data_;

    void check(size_t pos) const;
};
