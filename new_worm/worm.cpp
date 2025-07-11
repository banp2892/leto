// worm.cpp
#include "worm.h"
#include <windows.h>

std::string get_own_path() {
    char buffer[MAX_PATH];
    DWORD result = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (result == 0 || result == MAX_PATH)
        return "";
    return std::string(buffer);
}

std::string get_drive_root_from_path(const std::string& full_path) {
    if (full_path.size() < 3 || full_path[1] != ':' || full_path[2] != '\\')
        return "";
    return full_path.substr(0, 3);

}

bool is_hidden(const fs::path& path) {
    DWORD attrs = GetFileAttributesW(path.wstring().c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false; // ≈сли не удалось получить атрибуты, считаем что не скрыта
    }
    return (attrs & FILE_ATTRIBUTE_HIDDEN) || (attrs & FILE_ATTRIBUTE_SYSTEM);
}


std::string generate_random_suffix() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen));
}

// √енератор временной метки
std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count() % 10000;
    return std::to_string(ms);
}

void init_locale() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    std::locale::global(std::locale("en_US.UTF-8"));
#endif
}

std::wstring get_filename_stem(const fs::path& filepath) {
    try {
        return filepath.stem().wstring();
    }
    catch (...) {
        // ≈сли не удалось получить им€, возвращаем пустую строку
        return L"file";
    }
}