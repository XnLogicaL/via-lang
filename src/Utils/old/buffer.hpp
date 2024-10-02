#pragma once

#include <stdexcept>
#include <memory>
#include <string>
#include <iostream>
#include <cstdlib>   // For malloc and free
#include <cstdint>   // For uint8_t

void crash(const std::string& msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

class Buffer
{
public:
    Buffer(size_t bytes, bool readonly)
        : ptr(std::malloc(bytes)), bytes(bytes), readonly(readonly)
    {
        if (!ptr)
            crash("Failed to allocate memory for buffer");
    }

    ~Buffer()
    {
        free(ptr);
    }

    void write_bit(size_t offset = 0, bool value)
    {
        if (offset >= bytes * 8)
            crash("Buffer overflow");

        uint8_t* buf_ptr = static_cast<uint8_t*>(ptr);
        size_t byte_offset = offset / 8;
        size_t bit_offset = offset % 8;

        if (value)
            buf_ptr[byte_offset] |= (1 << bit_offset);
        else
            buf_ptr[byte_offset] &= ~(1 << bit_offset);
    }

    void write(size_t offset = 0, intmax_t value = 0) { _write<intmax_t>(offset, value); }

    void write_u8(size_t offset = 0, uint8_t value = 0) { _write<uint8_t>(offset, value); }
    void write_u16(size_t offset = 0, uint16_t value = 0) { _write<uint16_t>(offset, value); }
    void write_u32(size_t offset = 0, uint32_t value = 0) { _write<uint32_t>(offset, value); }

    void write_i8(size_t offset = 0, int8_t value = 0) { _write<int8_t>(offset, value); }
    void write_i16(size_t offset = 0, int16_t value = 0) { _write<int16_t>(offset, value); }
    void write_i32(size_t offset = 0, int32_t value = 0) { _write<int32_t>(offset, value); }

    void write_f32(size_t offset = 0, float value = 0) { _write_float<float>(offset, value); }
    void write_f64(size_t offset = 0, double value = 0) { _write_float<double>(offset, value); }

    void write_c_str(size_t offset = 0, const char* str = nullptr)
    {
        if (readonly)
            crash("Attempt to write to read-only buffer");

        if (str == nullptr)
            return;

        size_t len = strlen(str);

        if (offset + len + 1 > bytes)
            crash("Buffer overflow when writing C-style string");

        char* buf_ptr = static_cast<char*>(ptr);
        memcpy(&buf_ptr[offset], str, len + 1);
    }

    intmax_t read(size_t offset = 0) { return _read<intmax_t>(offset); }

    uint8_t read_u8(size_t offset = 0) { return _read<uint8_t>(offset); }
    uint16_t read_u16(size_t offset = 0) { return _read<uint16_t>(offset); }
    uint32_t read_u32(size_t offset = 0) { return _read<uint32_t>(offset); }

    int8_t read_i8(size_t offset = 0) { return _read<int8_t>(offset); }
    int16_t read_i16(size_t offset = 0) { return _read<int16_t>(offset); }
    int32_t read_i32(size_t offset = 0) { return _read<int32_t>(offset); }

    float read_f32(size_t offset = 0) { return _read_float<float>(offset); }
    double read_f64(size_t offset = 0) { return _read_float<double>(offset); }

    char* c_str()
    {
        return static_cast<char*>(ptr);
    }

private:
    size_t bytes;
    void* ptr;
    bool readonly;

    template <typename _Ty>
    void _write(size_t _Offset, _Ty _Value) const
    {
        if (readonly)
            crash("Attempt to write to read-only buffer");

        if (_Offset >= bytes / sizeof(_Ty))
            crash("Buffer overflow");

        _Ty* buf_ptr = static_cast<_Ty*>(ptr);
        buf_ptr[_Offset] = _Value;
    }

    template <typename _Ty>
    void _write_float(size_t _Offset, _Ty _Value) const
    {
        if (readonly)
            crash("Attempt to write to read-only buffer");

        if (_Offset + sizeof(_Ty) > bytes)
            crash("Buffer overflow");

        _Ty* buf_ptr = static_cast<_Ty*>(ptr);
        buf_ptr[_Offset / sizeof(_Ty)] = _Value;
    }

    template <typename _Ty>
    _Ty _read(size_t _Offset) const
    {
        if (_Offset >= bytes / sizeof(_Ty))
            crash("Buffer overflow");

        _Ty* buf_ptr = static_cast<_Ty*>(ptr);
        return buf_ptr[_Offset];
    }

    template <typename _Ty>
    _Ty _read_float(size_t _Offset) const
    {
        if (_Offset + sizeof(_Ty) > bytes)
            crash("Buffer overflow");

        _Ty* buf_ptr = static_cast<_Ty*>(ptr);
        return buf_ptr[_Offset / sizeof(_Ty)];
    }
};
