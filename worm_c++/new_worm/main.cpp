#include "worm.h"

int main() {
	
	setlocale(LC_ALL, "Russian");
	worm worm1;
	//worm worm1("E:\\test_worm", 1);
	worm1.search_list_dir();
	worm1.print_list_dir();
	//worm1.copy_file();
	wcout<<get_own_path();
}