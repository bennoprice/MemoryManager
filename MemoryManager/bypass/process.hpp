#pragma once
#include <memory>
#include "driver.hpp"
#include "system.hpp"
#include "native.hpp"

namespace bypass
{
	class process
	{
	private:
		using driver_t = std::shared_ptr<driver>;
		using system_t = std::shared_ptr<system>;
	public:
		explicit process(native::pid_t proc_id, driver_t driver, system_t system);

		native::virt_addr get_module_base() const;
		native::virt_addr get_dirbase() const;

		template <typename T>
		T rpm(native::virt_addr virt_addr) const
		{
			return _driver->read_virtual_memory<T>(_dirbase, virt_addr);
		}

		template <typename T>
		void wpm(native::virt_addr virt_addr, T val) const
		{
			_driver->write_virtual_memory<T>(_dirbase, virt_addr, val);
		}
	private:
		driver_t _driver;
		system_t _system;

		native::pid_t _pid;
		native::virt_addr _eprocess;
		std::uint64_t _dirbase;
		std::uint64_t _module_base;
	};
}