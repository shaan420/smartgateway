#include "RuleManager.hpp"
#include <iostream>

int main()
{
	GMainLoop *mainloop = NULL;

	RULE_MANAGER->Init();

	mainloop = g_main_loop_new(NULL, FALSE);

	g_print("starting main-loop.\n");
	g_main_loop_run(mainloop);
	return 0;
}
