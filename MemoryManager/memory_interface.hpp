#pragma once
#include <functional>
#include "bypass/driver.hpp"
#include "bypass/system.hpp"
#include "bypass/process.hpp"
#include "handle.hpp"

namespace memory_interface
{
	class memory
	{
	private:
		struct proc_info
		{
			std::wstring name;
			std::uint64_t base_addr;
			std::uint32_t id;
		};
	public:
		explicit memory() = default;
		void attach(std::uint32_t proc_id, std::wstring_view proc_name, bool use_driver);
		void on_attach(std::function<void()> action);

		std::shared_ptr<usermode::handle> get_user_process() const;
		std::shared_ptr<bypass::process> get_kernel_process() const;
		std::shared_ptr<bypass::driver> get_driver();
		std::shared_ptr<bypass::system> get_system();

		bool is_attached() const;
		bool is_using_driver() const;
		proc_info get_proc_info() const;

		template <typename T>
		T rpm(std::uint64_t addr) const
		{
			if (!is_attached())
				return 0;

			return (_user_process == nullptr ? get_kernel_process()->rpm<T>(addr) : get_user_process()->rpm<T>(addr));
		}

		template <typename T>
		void wpm(std::uint64_t addr, T val) const
		{
			if (!is_attached())
				return;
			_user_process == nullptr ? get_kernel_process->wpm<T>(addr, val) : get_user_process()->wpm<T>(addr, val);
		}
	private:
		std::shared_ptr<usermode::handle> _user_process = nullptr;
		std::shared_ptr<bypass::driver> _driver = nullptr;
		std::shared_ptr<bypass::system> _system = nullptr;
		std::shared_ptr<bypass::process> _kernel_process = nullptr;

		std::vector<std::function<void()>> _on_attach_events;
		proc_info _proc_info;
	};
}