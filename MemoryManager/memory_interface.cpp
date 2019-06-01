#include <Windows.h>
#include <TlHelp32.h>
#include <sstream>
#include "menus/menus.hpp"
#include "memory_interface.hpp"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace memory_interface
{
	memory::proc_info::proc_info(std::wstring_view proc_name, std::uint64_t proc_base_addr, std::uint32_t proc_id)
	{
		char buf[MAX_PATH];
		wcstombs_s(nullptr, buf, proc_name.data(), sizeof(buf));
		name = buf;

		std::stringstream stream;
		stream << "0x" << std::hex << proc_base_addr;
		base_addr = stream.str();

		id = std::to_string(proc_id);
	}

	void memory::attach(std::uint32_t proc_id, std::wstring_view proc_name, bool use_driver)
	{
		_user_process = nullptr;
		_kernel_process = nullptr;

		std::uint64_t base_addr;

		if (use_driver)
		{
			if (!get_driver()->is_init())
			{
				global::logger->log("[error] failed to attach to process");
				return;
			}
			_kernel_process = std::make_shared<bypass::process>(proc_id, get_driver(), get_system());
			base_addr = _kernel_process->get_module_base();
			global::logger->log("[info] kernel attached to process: %d", proc_id);
		}
		else
		{
			_user_process = std::make_shared<usermode::handle>(proc_id);
			base_addr = _user_process->get_module_base(proc_name);
			global::logger->log("[info] usermode attached to process: %d", proc_id);
		}

		_proc_info = proc_info(proc_name, base_addr, proc_id);
	}

	std::shared_ptr<usermode::handle> memory::get_user_process() const
	{
		return _user_process;
	}

	std::shared_ptr<bypass::process> memory::get_kernel_process() const
	{
		return _kernel_process;
	}

	std::shared_ptr<bypass::driver> memory::get_driver()
	{
		if (_driver == nullptr)
			_driver = std::make_shared<bypass::driver>();
		return _driver;
	}

	std::shared_ptr<bypass::system> memory::get_system()
	{
		if (_system == nullptr)
			_system = std::make_shared<bypass::system>(get_driver());
		return _system;
	}

	bool memory::is_attached() const
	{
		return (_user_process != nullptr || _kernel_process != nullptr);
	}

	bool memory::is_using_driver() const
	{
		return (_kernel_process != nullptr);
	}

	memory::proc_info memory::get_proc_info() const
	{
		return _proc_info;
	}
}