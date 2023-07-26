#ifndef UTILS_HPP
#define UTILS_HPP

#include "string"
#include "../External/json.hpp"
using json = nlohmann::json;

// 模板函数不能放在cpp里，否则会找不到符号
template <typename T>
json arrayToJson(T* arr, int len)
{
	json j;
	for (int i = 0; i < len; ++i)
		j += arr[i];
	return j;
}

template <typename T>
int jsonToArray(const json& j, T* buf, int bufSize)
{
	int count = 0;
	if (!j.is_array() || bufSize <= 0)
		throw "Must be array";

	for (auto it : j) {
		buf[count++] = it;
		if (count >= bufSize) break;
	}
	return count;
}

// 引用windows.h的函数需要放在cpp里，避免污染全局命名空间
std::string string_To_UTF8(const std::string& str);
std::string UTF8_To_string(const std::string& str);
#endif