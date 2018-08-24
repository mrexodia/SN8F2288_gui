#pragma once

#include <cstdint>
#include <cstddef>

template<class T>
struct ArrayFunctions
{
    typedef T(*ReadFunc)(void*, uint16_t);
    typedef void(*WriteFunc)(void*, uint16_t, T);

    T read(uint16_t addr)
    {
        return _read(_thisptr, addr);
    }

    void write(uint16_t addr, T value)
    {
        return _write(_thisptr, addr, value);
    }

    void* _thisptr;
    ReadFunc _read;
    WriteFunc _write;

    T getBit(uint16_t index, uint8_t bitpos)
    {
        auto value = read(index);
        return value >> bitpos & 1;
    }

    void setBit(uint16_t index, uint8_t bitpos, T value)
    {
        auto mask = uint8_t(1 << bitpos);
        value <<= bitpos;
        value &= mask;
        auto newValue = read(index);
        newValue &= ~mask;
        newValue |= value;
        write(index, newValue);
    }

    ArrayFunctions(void* thisptr, ReadFunc read, WriteFunc write) : _thisptr(thisptr), _read(read), _write(write) { }
};

template<class T, size_t Count>
class BaseArray
{
    T data[Count];

public:
    BaseArray()
    {
        memset(data, 0, sizeof(data));
    }

    virtual T get(uint16_t index)
    {
        return data[index];
    }

    virtual void set(uint16_t index, T value)
    {
        data[index] = value;
    }

    static const size_t count = Count;
};

template<uint16_t ByteIndex, uint8_t BitIndex = 0, uint8_t SizeInBits = 8>
class Register
{
    ArrayFunctions<uint8_t> & ram;

public:
    explicit Register(ArrayFunctions<uint8_t> & ram) : ram(ram) { }

    uint16_t get(ArrayFunctions<uint8_t> & ram) const
    {
        uint16_t result = 0;
        if(BitIndex + SizeInBits > 8)
            result = ram.read(ByteIndex) | ram.read(ByteIndex + 1) << 8;
        else
            result = ram.read(ByteIndex);
        result >>= BitIndex;
        auto mask = uint16_t(~uint16_t(int16_t(0x8000) >> (15 - SizeInBits)));
        result &= mask;
        return result;
    }

    uint16_t get() const
    {
        return get(ram);
    }

    void set(ArrayFunctions<uint8_t> & ram, uint16_t value)
    {
        uint16_t result = 0;
        if(BitIndex + SizeInBits > 8)
            result = ram.read(ByteIndex) | ram.read(ByteIndex + 1) << 8;
        else
            result = ram.read(ByteIndex);
        auto mask = uint16_t(~uint16_t(int16_t(0x8000) >> (15 - SizeInBits)));
        value &= mask;
        value <<= BitIndex;
        mask = ~(mask << BitIndex);
        result &= mask;
        result |= value;
        ram.write(ByteIndex, uint8_t(result & 0xFF));
        if(BitIndex + SizeInBits > 8)
            ram.write(ByteIndex + 1, uint8_t(result >> 8));
    }

    void set(uint16_t value)
    {
        set(ram, value);
    }

    const static uint16_t index = ByteIndex;
};
