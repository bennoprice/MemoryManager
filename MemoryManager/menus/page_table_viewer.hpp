#pragma once
#include <cstdint>
#include <memory>
#include <array>

namespace menus
{
	namespace page_table_viewer
	{
		namespace memory
		{
			typedef union _entry_addr
			{
				uint64_t value;
				struct
				{
					uint64_t present : 1;
					uint64_t rw : 1;
					uint64_t user : 1;
					uint64_t write_through : 1;
					uint64_t cache_disable : 1;
					uint64_t accessed : 1;
					uint64_t dirty : 1;
					uint64_t pat : 1;
					uint64_t global : 1;
					uint64_t ignored_1 : 3;
					uint64_t page_frame : 40;
					uint64_t ignored_3 : 11;
					uint64_t xd : 1;
				};
			} entry_addr;

			class page_table;

			class page_table_entry
			{
			public:
				explicit page_table_entry(entry_addr);
				entry_addr get_addr() const;
				std::unique_ptr<page_table> get_page_table();
			private:
				entry_addr _addr;
			};

			class page_table
			{
			public:
				explicit page_table(std::uint64_t addr);
				std::size_t get_size() const;
				std::shared_ptr<page_table_entry> get_page_table_entry(int idx) const;
			private:
				std::array<entry_addr, 512> _entry_addrs;
			};
		}
	}
}