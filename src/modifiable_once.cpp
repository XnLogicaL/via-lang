/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "modifiable_once.h"

namespace via::utils {

template<typename T>
void ModifiableOnce<T>::set(T new_value)
{
    if (has_modified) {
        return;
    }

    has_modified = true;
    value = new_value;
}

template<typename T>
T ModifiableOnce<T>::get()
{
    return value;
}

template<typename T>
const T &ModifiableOnce<T>::get() const
{
    return value;
}

} // namespace via::utils
