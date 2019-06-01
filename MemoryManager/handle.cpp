#include <Windows.h>
#include <TlHelp32.h>
#include "handle.hpp"
#include "menus/menus.hpp"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace usermode
{
	handle::handle(std::uint32_t proc_id)
		: _pid(proc_id)
	{
		_handle = OpenProcess(PROCESS_ALL_ACCESS, true, _pid);
		if (_handle == INVALID_HANDLE_VALUE)
		{
			global::logger->log("[error] failed to get process handle");
			return;
		}
	}

	std::uint64_t handle::get_module_base(std::wstring_view mod_name) const
	{
		auto snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _pid);
		auto mod = MODULEENTRY32W{ sizeof(MODULEENTRY32W) };

		if (::Module32First(snapshot, &mod))
		{
			do {
				if (!_wcsicmp(mod_name.data(), mod.szModule))
				{
					::CloseHandle(snapshot);
					return reinterpret_cast<std::uint64_t>(mod.modBaseAddr);
				}
			} while (::Module32Next(snapshot, &mod));
		}
		::CloseHandle(snapshot);
		return 0;
	}
}