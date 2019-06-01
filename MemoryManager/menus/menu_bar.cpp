#include <memory>
#include "menus.hpp"
#include "../util.hpp"
#include "../config.hpp"
#include "../imgui/imgui.h"
#include "../memory_interface.hpp"

namespace global
{
	extern std::unique_ptr<memory_interface::memory> mem;
}

namespace menus
{
	namespace menu_bar
	{
		void render()
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("file"))
				{
					if (ImGui::BeginMenu("attach"))
					{
						static ImGuiTextFilter filter;
						static auto use_driver = false;

						ImGui::Checkbox("use driver", &use_driver);
						filter.Draw("filter");
						ImGui::Separator();
						ImGui::BeginChild("processes", {300, 500});

						util::iterate_processes([](PROCESSENTRY32W proc)
						{
							char buf[MAX_PATH];
							wcstombs_s(nullptr, buf, proc.szExeFile, sizeof(buf));

							if (filter.IsActive())
								if (!filter.PassFilter(buf))
									return;

							if (ImGui::MenuItem(buf))
							{
								global::mem->attach(proc.th32ProcessID, proc.szExeFile, use_driver);
								ImGui::CloseCurrentPopup();
							}
						});

						ImGui::EndChild();
						ImGui::EndMenu();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("open")) {}
					if (ImGui::MenuItem("save", "ctrl+s")) {}
					if (ImGui::MenuItem("save as")) {}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("windows"))
				{
					ImGui::MenuItem("memory dumper", 0, &config::show_memory_dumper);
					ImGui::MenuItem("structure viewer", 0, &config::show_structure_viewer);
					ImGui::MenuItem("page table viewer", 0, &config::show_page_table_viewer);
					ImGui::Separator();
					ImGui::MenuItem("debug log", 0, &config::show_debug_log);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("settings"))
				{
					ImGui::ColorEdit3("background colour", (float*)&config::settings::background_colour, ImGuiColorEditFlags_NoInputs);
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}
	}
}