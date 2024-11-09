#ifndef VIA_DEBOUNCE_H
#define VIA_DEBOUNCE_H

namespace via
{

namespace util
{

class debounce
{
    bool val;

public:

    debounce(bool v) : val(v) {}

    bool get()
    {
        bool vclone = val;
        val = !val;
        return vclone;
    }
};

} // namespace util

} // namespace via

#endif // VIA_DEBOUNCE_H
