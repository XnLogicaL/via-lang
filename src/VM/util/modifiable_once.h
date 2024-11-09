#pragma once

namespace via
{

namespace util
{
    
template <typename T>
class modifiable_once
{
    bool has_modified;
    T value;

public:

    modifiable_once(T value)
        : has_modified(false)
        , value(value) {}
    
    void set(T new_value)
    {
        if (has_modified)
        {
            return;
        }

        has_modified = true;
        value = new_value;
    }

    T get()
    {
        return value;
    }
};

} // namespace util

} // namespace via
