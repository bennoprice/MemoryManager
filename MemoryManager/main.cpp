#include <Windows.h>
#include "memory_interface.hpp"
#include "menus/menus.hpp"
#include "config.hpp"
#include "gui.hpp"

// add filter to attach to process
// show process icons
// make overlay show attached process information
// make driver also use pml4 class from page_table_viewer during linear address translation
// change proc_info, make class maybe

namespace global
{
	std::unique_ptr<gui::menu> menu;
	std::unique_ptr<memory_interface::memory> mem;
	std::unique_ptr<menus::debug_log::logger> logger;
}

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd, int show_cmd)
{
	global::logger = std::make_unique<menus::debug_log::logger>();
	global::mem = std::make_unique<memory_interface::memory>();
	global::menu = std::make_unique<gui::menu>(L"Memory Manager");

	while (
		global::menu->render([]()
		{
			menus::menu_bar::render();
			menus::overlay::render();
			menus::debug_log::render();
			menus::memory_dumper::render();
			menus::structure_viewer::render();
			menus::page_table_viewer::render();

			//ImGui::ShowDemoWindow();
		})
	);
}