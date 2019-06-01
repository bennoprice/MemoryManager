#include <Windows.h>
#include <TlHelp32.h>
#include <cinttypes>
#include "process.hpp"
#include "../menus/menus.hpp"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace bypass
{
	process::process(native::pid_t proc_id, driver_t driver, system_t system)
		: _pid(proc_id)
		, _driver(driver)
		, _system(system)
	{
		// get eprocess
		_eprocess = _system->get_eprocess(_pid);
		if (!_eprocess)
		{
			global::logger->log("[error] failed to get eprocess");
			return;
		}
		global::logger->log("[info] found eprocess: %#08x", _eprocess);

		// get dirbase
		_dirbase = _system->get_dirbase(_eprocess);
		if (!_dirbase)
		{
			global::logger->log("[error] failed to get process dirbase");
			return;
		}
		global::logger->log("[info] found process dirbase: %#08x", _dirbase);

		// get base address
		_module_base = _system->get_module_base(_eprocess);
		if (!_module_base)
		{
			global::logger->log("[error] failed to get base address");
			return;
		}
		global::logger->log("[info] found process base address: %#08x", _module_base);
	}

	native::virt_addr process::get_module_base() const
	{
		return _module_base;
	}

	native::virt_addr process::get_dirbase() const
	{
		return _dirbase;
	}
}