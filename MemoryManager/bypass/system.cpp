#include <Windows.h>
#include <Psapi.h>
#include "system.hpp"
#include "driver.hpp"
#include "../menus/menus.hpp"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace bypass
{
	system::system(driver_t driver)
		: _driver(driver)
	{ }

	native::virt_addr system::get_kernel_base() const
	{
		native::RTL_PROCESS_MODULES module_info;
		native::NtQuerySystemInformation(native::SystemModuleInformation, &module_info, sizeof(module_info), nullptr);

		return reinterpret_cast<native::virt_addr>(module_info.Modules[0].ImageBase);
	}

	native::virt_addr system::get_driver_base(std::string_view name) const
	{
		void* drivers[1024] = { 0 };
		::EnumDeviceDrivers(drivers, sizeof(drivers), 0);

		for (int i = 0; drivers[i]; ++i)
		{
			char driver_name[MAX_PATH];
			::GetDeviceDriverBaseNameA(drivers[i], driver_name, MAX_PATH);

			if (!::_stricmp(name.data(), driver_name))
				return reinterpret_cast<std::uint64_t>(drivers[i]);
		}
		return 0;
	}

	native::virt_addr system::find_kernel_proc(std::string_view name) const
	{
		auto ntoskrnl = ::LoadLibraryW(L"ntoskrnl.exe");
		auto krnl_base = get_kernel_base(); // could replace with get_driver_base("ntoskrnl.exe")
		auto proc_address = reinterpret_cast<native::virt_addr>(::GetProcAddress(ntoskrnl, name.data()));

		return (proc_address - reinterpret_cast<native::virt_addr>(ntoskrnl) + krnl_base);
	}

	native::virt_addr system::get_eprocess(native::pid_t pid) const
	{
		auto system_proc = find_kernel_proc("PsInitialSystemProcess");
		auto eprocess = _driver->read_system_memory<std::uint64_t>(system_proc);

		auto list_head = eprocess + eprocess_offset::ActiveProcessLinks;
		auto last_link = _driver->read_system_memory<std::uint64_t>(list_head + sizeof(std::uint64_t));
		auto cur_link = list_head;

		do
		{
			auto entry = cur_link - eprocess_offset::ActiveProcessLinks;
			auto unique_pid = _driver->read_system_memory<std::uint64_t>(entry + eprocess_offset::UniqueProcessId);

			if (pid == unique_pid)
				return entry;

			cur_link = _driver->read_system_memory<std::uint64_t>(cur_link);
		} while (cur_link != last_link);

		return 0;
	}

	native::virt_addr system::get_dirbase(native::virt_addr eprocess) const
	{
		return _driver->read_system_memory<native::virt_addr>(eprocess + eprocess_offset::DirectoryTableBase);
	}

	native::virt_addr system::get_module_base(native::virt_addr eprocess) const
	{
		return _driver->read_system_memory<native::virt_addr>(eprocess + eprocess_offset::SectionBaseAddress);
	}
}