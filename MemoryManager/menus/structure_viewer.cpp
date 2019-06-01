#include "menus.hpp"
#include "../config.hpp"
#include "../imgui/imgui.h"

namespace menus
{
	namespace structure_viewer
	{
		void render()
		{
			if (!config::show_structure_viewer)
				return;

			ImGui::SetNextWindowSize({ 350, 400 }, ImGuiCond_FirstUseEver);
			ImGui::Begin("structure viewer", &config::show_structure_viewer);
			ImGui::Text("this is some useful text.");

			ImGui::Text("application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	}
}