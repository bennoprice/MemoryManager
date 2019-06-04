#include <sstream>
#include <iomanip>
#include <string_view>
#include "menus.hpp"
#include "../config.hpp"
#include "../imgui/imgui.h"
#include "page_table_viewer.hpp"
#include "../memory_interface.hpp"

namespace global
{
	extern std::unique_ptr<memory_interface::memory> mem;
	extern std::unique_ptr<menus::debug_log::logger> logger; // could remove
}

namespace ImGui
{
	bool TreeNodeC(int prefix, std::uint64_t val)
	{
		std::stringstream index_stream;
		index_stream << std::setfill('0') << std::setw(3) << prefix;

		std::stringstream stream;
		stream << index_stream.str() << ": 0x" << std::hex << val;
		return TreeNode(std::to_string(prefix).c_str(), stream.str().c_str());
	}

	void TextC(std::uint64_t val, std::string_view prefix = "")
	{
		std::stringstream stream;
		if (prefix != "")
			stream << prefix << ": ";
		stream << "0x" << std::hex << val;
		Text(stream.str().c_str());
	}
}

namespace menus
{
	namespace page_table_viewer
	{
		namespace memory
		{
			//
			// page_table_entry
			//
			page_table_entry::page_table_entry(entry_addr addr)
				: _addr(addr)
			{ }

			entry_addr page_table_entry::get_addr() const
			{
				return _addr;
			}

			std::unique_ptr<page_table> page_table_entry::get_page_table()
			{
				return std::make_unique<page_table>(PFN_TO_PAGE(_addr.page_frame));
			}

			//
			// page_table
			//
			page_table::page_table(std::uint64_t addr)
				: _base_addr(addr)
			{
				_entry_addrs = global::mem->get_driver()->read_physical_memory<std::array<entry_addr, 512>>(_base_addr);
			}

			std::size_t page_table::get_size() const
			{
				return _entry_addrs.size();
			}

			std::shared_ptr<page_table_entry> page_table::get_page_table_entry(int idx) const
			{
				return std::make_unique<page_table_entry>(_entry_addrs[idx]);
			}

			void page_table::set_page_table_entry(int idx, entry_addr addr) const
			{
				global::mem->get_driver()->write_physical_memory<std::uint64_t>(_base_addr + idx * sizeof(std::uint64_t), addr.value);
			}
		}

		void iterate_page_table_entries(std::uint64_t page_table_addr)
		{
			auto page_table = std::make_unique<memory::page_table>(page_table_addr);

			for (auto i = 0u; i < page_table->get_size(); ++i)
			{
				auto pte = page_table->get_page_table_entry(i);
				auto pte_addr = pte->get_addr();

				if (ImGui::TreeNodeC(i, pte_addr.value))
				{
					std::array<std::pair<std::string_view, std::uint64_t>, 13> pml4e_info =
					{
						{
							{ "present", pte_addr.present},
							{ "rw", pte_addr.rw },
							{ "user", pte_addr.user },
							{ "write_through", pte_addr.write_through },
							{ "cache_disable", pte_addr.cache_disable },
							{ "accessed", pte_addr.accessed },
							{ "dirty", pte_addr.dirty },
							{ "pat", pte_addr.pat },
							{ "global", pte_addr.global },
							{ "ignored_1", pte_addr.ignored_1 },
							{ "page_frame", pte_addr.page_frame },
							{ "ignored_3", pte_addr.ignored_3 },
							{ "xd", pte_addr.xd }
						}
					};

					ImGui::Columns(2, "page_table_entry", true);
					for (auto j = 0u; j < pml4e_info.size(); ++j)
					{
						auto val = pml4e_info[j];

						ImGui::Text(val.first.data());
						ImGui::NextColumn();
						ImGui::TextC(val.second);

						static char text_edit[17];
						if (ImGui::BeginPopup(std::to_string(j).c_str()))
						{
							ImGui::InputText("##val_edit", text_edit, sizeof(text_edit), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsNoBlank);
							ImGui::SameLine();
							if (ImGui::Button("apply"))
							{
								auto val_edit = static_cast<std::uint64_t>(strtol(text_edit, 0, 16));

								switch (j)
								{
								case 0:
									pte_addr.present = val_edit;
									break;
								case 1:
									pte_addr.rw = val_edit;
									break;
								case 2:
									pte_addr.user = val_edit;
									break;
								case 3:
									pte_addr.write_through = val_edit;
									break;
								case 4:
									pte_addr.cache_disable = val_edit;
									break;
								case 5:
									pte_addr.accessed = val_edit;
									break;
								case 6:
									pte_addr.dirty = val_edit;
									break;
								case 7:
									pte_addr.pat = val_edit;
									break;
								case 8:
									pte_addr.global = val_edit;
									break;
								case 9:
									pte_addr.ignored_1 = val_edit;
									break;
								case 10:
									pte_addr.page_frame = val_edit;
									break;
								case 11:
									pte_addr.ignored_3 = val_edit;
									break;
								case 12:
									pte_addr.xd = val_edit;
									break;
								}

								page_table->set_page_table_entry(i, pte_addr);
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}

						if (ImGui::IsItemClicked(1))
						{
							std::stringstream stream;
							stream << std::hex << val.second;
							auto size = stream.str().copy(text_edit, stream.str().length());
							text_edit[size] = '\0';

							ImGui::OpenPopup(std::to_string(j).c_str());
						}
						ImGui::NextColumn();
					}

					ImGui::Columns(1);

					auto opos = ImGui::GetCursorPos().x;
					ImGui::SetCursorPosX(opos - 21.f);
					if (ImGui::TreeNode("page_table"))
					{
						iterate_page_table_entries(PFN_TO_PAGE(pte_addr.page_frame));
						ImGui::TreePop();
					}
					ImGui::SetCursorPosX(opos);

					ImGui::TreePop();
					ImGui::Spacing();
					//ImGui::Separator();
				}
			}
		}

		void render()
		{
			if (!config::show_page_table_viewer)
				return;

			ImGui::SetNextWindowSize({ 500, 600 }, ImGuiCond_FirstUseEver);
			ImGui::Begin("page table viewer", &config::show_page_table_viewer);

			if (global::mem->get_kernel_process() == nullptr)
			{
				ImGui::Text("you are currently not attached to a process using the driver");
				ImGui::Spacing();
				ImGui::Text("1) go to 'file -> attach' and check 'use driver'");
				ImGui::Text("2) select a process to attach to");
				return;
			}

			static memory::entry_addr cr3 = { global::mem->get_kernel_process()->get_dirbase() };
			iterate_page_table_entries(PFN_TO_PAGE(cr3.page_frame));

			ImGui::End();
		}
	}
}