#pragma once

#include <format>
#include <functional>
#include <string>
#include <vector>

template<typename T>
std::string format_vector(const std::vector<T> &vec, std::function<std::string(const T)> func);