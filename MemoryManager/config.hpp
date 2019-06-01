#pragma once
#include "imgui/imgui.h"

namespace config
{
	namespace settings
	{
		inline auto background_colour = ImVec4(0.349f, 0.675f, 0.541f, 1.f);
	}
	
	inline auto show_memory_dumper = false;
	inline auto show_structure_viewer = false;
	inline auto show_page_table_viewer = false;
	inline auto show_debug_log = true;
}