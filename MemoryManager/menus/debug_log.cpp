#include <memory>
#include "menus.hpp"
#include "../config.hpp"
#include "../imgui/imgui.h"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace menus
{
	namespace debug_log
	{
		logger::logger()
		{
			clear();
		}

		void logger::log(const char* fmt, ...) IM_FMTARGS(2)
		{
			auto old_size = _buf.size();
			va_list args;
			_buf.appendf("[%05d] ", count);
			va_start(args, fmt);
			_buf.appendfv(fmt, args);
			va_end(args);
			_buf.append("\n");
			for (auto new_size = _buf.size(); old_size < new_size; old_size++)
				if (_buf[old_size] == '\n')
					_line_offsets.push_back(old_size + 1);
			if (_auto_scroll)
				_scroll_to_bottom = true;
			++count;
		}

		void logger::clear()
		{
			_buf.clear();
			_line_offsets.clear();
			_line_offsets.push_back(0);
		}

		void logger::draw()
		{
			if (ImGui::BeginPopup("options"))
			{
				if (ImGui::Checkbox("auto-scroll", &_auto_scroll))
					if (_auto_scroll)
						_scroll_to_bottom = true;
				ImGui::EndPopup();
			}

			if (ImGui::Button("options"))
				ImGui::OpenPopup("options");
			ImGui::SameLine();
			if (ImGui::Button("clear")) clear();
			ImGui::SameLine();
			_filter.Draw("filter", -100.0f);

			ImGui::Separator();
			ImGui::BeginChild("scrolling", { 0, 0 }, false, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			if (_filter.IsActive())
			{
				for (int line_no = 0; line_no < _line_offsets.Size; line_no++)
				{
					const char* line_start = _buf.begin() + _line_offsets[line_no];
					const char* line_end = (line_no + 1 < _line_offsets.Size) ? (_buf.begin() + _line_offsets[line_no + 1] - 1) : _buf.end();
					if (_filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else
			{
				ImGuiListClipper clipper;
				clipper.Begin(_line_offsets.Size);
				while (clipper.Step())
				{
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
					{
						const char* line_start = _buf.begin() + _line_offsets[line_no];
						const char* line_end = (line_no + 1 < _line_offsets.Size) ? (_buf.begin() + _line_offsets[line_no + 1] - 1) : _buf.end();
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (_scroll_to_bottom)
				ImGui::SetScrollHereY(1.0f);
			_scroll_to_bottom = false;

			ImGui::EndChild();
		}

		void render()
		{
			if (!config::show_debug_log)
				return;

			ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
			ImGui::Begin("debug log", &config::show_debug_log);
			global::logger->draw();
			ImGui::End();
		}
	}
}