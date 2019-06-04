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
	void memory::attach(std::uint32_t proc_id, std::wstring_view proc_name, bool use_driver)
	{
		_user_process = nullptr;
		_kernel_process = nullptr;

		if (use_driver)
		{
			if (!get_driver()->is_init())
			{
				global::logger->log("[error] failed to attach to process");
				return;
			}
			_kernel_process = std::make_shared<bypass::process>(proc_id, get_driver(), get_system());
			_proc_info.base_addr = _kernel_process->get_module_base();
			global::logger->log("[info] kernel attached to process: %d", proc_id);
		}
		else
		{
			_user_process = std::make_shared<usermode::handle>(proc_id);
			_proc_info.base_addr = _user_process->get_module_base(proc_name);
			global::logger->log("[info] usermode attached to process: %d", proc_id);
		}

		_proc_info.id = proc_id;
		_proc_info.name = proc_name;

		for (auto& on_attach : _on_attach_events)
			on_attach();
	}

	void memory::on_attach(std::function<void()> action)
	{
		global::logger->log("on attach even registered");
		_on_attach_events.push_back(action);
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