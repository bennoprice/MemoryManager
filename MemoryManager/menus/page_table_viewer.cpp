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
}

namespace ImGui
{
	bool TreeNodeC(int prefix, std::uint64_t val)
	{
		std::stringstream index_stream;
		index_stream << std::setfill('0') << std::setw(3) << prefix;

		std::stringstream stream;
		stream << index_stream.str() << ": 0x" << std::hex << val;
		return TreeNode(stream.str().c_str());
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
			{
				_entry_addrs = global::mem->get_driver()->read_physical_memory<std::array<entry_addr, 512>>(addr);
			}

			std::size_t page_table::get_size() const
			{
				return _entry_addrs.size();
			}

			std::shared_ptr<page_table_entry> page_table::get_page_table_entry(int idx) const
			{
				return std::make_unique<page_table_entry>(_entry_addrs[idx]);
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
					for (auto val : pml4e_info)
					{
						ImGui::Text(val.first.data());
						ImGui::NextColumn();
						ImGui::TextC(val.second);
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