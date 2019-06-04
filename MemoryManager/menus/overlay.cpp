#include <memory>
#include <sstream>
#include "menus.hpp"
#include "../imgui/imgui.h"
#include "../memory_interface.hpp"

namespace global
{
	extern std::unique_ptr<memory_interface::memory> mem;
}

namespace menus
{
	namespace overlay
	{
		struct proc_info
		{
			std::string name;
			std::string id;
			std::string base_addr;
			std::string using_driver;
		};

		void render()
		{
			static proc_info attached_proc = { "none", "none", "none", "false" };
			static bool once = []()
			{
				global::mem->on_attach([]()
				{
					auto proc = global::mem->get_proc_info();

					// convert wstring to string
					char name_buf[MAX_PATH];
					wcstombs_s(nullptr, name_buf, proc.name.data(), sizeof(name_buf));
					attached_proc.name = name_buf;

					// convert uint64_t to string
					std::stringstream base_addr_stream;
					base_addr_stream << "0x" << std::hex << proc.base_addr;
					attached_proc.base_addr = base_addr_stream.str();

					// convert uint32_t to string
					attached_proc.id = std::to_string(proc.id);

					// convert bool to string
					attached_proc.using_driver = global::mem->is_using_driver() ? "true" : "false";
				});
				return true;
			} ();

			constexpr auto dist = 10.f;
			static auto corner = 3;
			static auto open = true;
			auto io = ImGui::GetIO();

			auto window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - dist : dist, (corner & 2) ? io.DisplaySize.y - dist : dist);
			auto window_pos_pivot = ImVec2((corner & 1) ? 1.f : 0.f, (corner & 2) ? 1.f : 0.f);
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

			ImGui::SetNextWindowSize({ 240.f, 112.f });
			ImGui::SetNextWindowBgAlpha(0.4f);
			if (ImGui::Begin("overlay", &open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				ImGui::Columns(2, "attached_proc_info", true);
				ImGui::Text("name"); ImGui::NextColumn(); ImGui::Text(attached_proc.name.c_str()); ImGui::NextColumn();
				ImGui::Text("process id"); ImGui::NextColumn(); ImGui::Text(attached_proc.id.c_str()); ImGui::NextColumn();
				ImGui::Text("base address"); ImGui::NextColumn(); ImGui::Text(attached_proc.base_addr.c_str()); ImGui::NextColumn();
				ImGui::Text("using driver"); ImGui::NextColumn(); ImGui::Text(attached_proc.using_driver.c_str());
				ImGui::Columns(1);


				ImGui::Separator();
				if (ImGui::IsMousePosValid())
					ImGui::Text("mouse pos: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
				else
					ImGui::Text("mouse pos: <invalid>");
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("top-left", 0, corner == 0)) corner = 0;
					if (ImGui::MenuItem("top-right", 0, corner == 1)) corner = 1;
					if (ImGui::MenuItem("bottom-left", 0, corner == 2)) corner = 2;
					if (ImGui::MenuItem("bottom-right", 0, corner == 3)) corner = 3;
					ImGui::EndPopup();
				}
			}
			ImGui::End();
		}
	}
}