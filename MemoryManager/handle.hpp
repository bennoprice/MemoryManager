#pragma once
#include <string_view>

namespace usermode
{
	class handle
	{
	public:
		explicit handle(std::uint32_t proc_id);

		std::uint64_t get_module_base(std::wstring_view mod_name) const;

		template <typename T>
		T rpm(std::uint64_t addr) const
		{
			T buf;
			ReadProcessMemory(_handle, reinterpret_cast<void*>(addr), &buf, sizeof(T), nullptr);
			return buf;
		}

		template <typename T>
		void wpm(std::uint64_t addr, T val) const
		{
			T buf = val;
			WriteProcessMemory(_handle, reinterpret_cast<void*>(addr), &buf, sizeof(T), nullptr);
		}
	private:
		HANDLE _handle;
		std::uint32_t _pid;
	};
}