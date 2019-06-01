#include <memory>
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
		void render()
		{
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
				auto attached = global::mem->is_attached();
				if (attached)
				{
					int hey = 10 + 5;
				}
				auto proc_info = global::mem->get_proc_info();

				ImGui::Columns(2, "attached_proc_info", true);
				ImGui::Text("name"); ImGui::NextColumn(); ImGui::Text(attached ? proc_info.name.c_str() : "none"); ImGui::NextColumn();
				ImGui::Text("process id"); ImGui::NextColumn(); ImGui::Text(attached ? proc_info.id.c_str() : "none"); ImGui::NextColumn();
				ImGui::Text("base address"); ImGui::NextColumn(); ImGui::Text(attached ? proc_info.base_addr.c_str() : "none"); ImGui::NextColumn();
				ImGui::Text("using driver"); ImGui::NextColumn(); ImGui::Text(global::mem->is_using_driver() ? "true" : "false");
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