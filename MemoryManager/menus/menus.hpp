#pragma once
#include "../imgui/imgui.h"

namespace menus
{
	namespace menu_bar
	{
		void render();
	}
	namespace overlay
	{
		void render();
	}
	namespace debug_log
	{
		class logger
		{
		public:
			explicit logger();
			void log(const char* fmt, ...);
			void clear();
			void draw();
		private:
			ImGuiTextBuffer _buf;
			ImGuiTextFilter _filter;
			ImVector<int> _line_offsets;
			int count = 1;
			bool _auto_scroll = true;
			bool _scroll_to_bottom = false;
		};
		void render();
	}
	namespace memory_dumper
	{
		void render();
	}
	namespace structure_viewer
	{
		void render();
	}
	namespace page_table_viewer
	{
		void render();
	}
}