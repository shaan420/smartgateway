#include "DataManager.hpp"
#include <iostream>

int main()
{
	GMainLoop *mainloop = NULL;
	DATA_MANAGER->Init();

	mainloop = g_main_loop_new(NULL, FALSE);
/*
	DATA_MANAGER->CreateNewStorageSlot("lighting1");

	DATA_MANAGER->Insert("lighting1", "Hello", 5);

	DATA_MANAGER->Find("lighting1", &value);

	cout << "Got value: " << value << endl;
*/
	g_print("starting main-loop.\n");
	g_main_loop_run(mainloop);
	return 0;
}
