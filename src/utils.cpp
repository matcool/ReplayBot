#include "utils.hpp"

std::string narrow(const wchar_t* str) {
    int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) { /* fuck */ }
    auto buffer = new char[size];
    WideCharToMultiByte(CP_UTF8, 0, str, -1, buffer, size, nullptr, nullptr);
    std::string result(buffer, size_t(size) - 1);
    delete[] buffer;
    return result;
}

std::wstring widen(const char* str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if (size <= 0) { /* fuck */ }
    auto buffer = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, buffer, size);
    std::wstring result(buffer, size_t(size) - 1);
    delete[] buffer;
    return result;
}