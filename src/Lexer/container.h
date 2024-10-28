#ifndef VIA_CONTAINER_H
#define VIA_CONTAINER_H

#include "common.h"
#include "token.h"

namespace via
{

namespace Tokenization
{

struct viaSourceContainer
{
    std::vector<Token> tokens;
    std::string source;
    std::string file_name;
};

} // namespace name
    
} // namespace via

#endif // VIA_CONTAINER_H