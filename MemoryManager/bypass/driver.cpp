#include <Windows.h>
#include "driver.hpp"
#include "loader.hpp"
#include <string>
#include "gdrv.hpp"
#include "../menus/menus.hpp"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace bypass
{
	driver::driver()
	{
		if (!loader::is_driver_loaded("gdrv.sys"))
		{
			// load driver
			if (!loader::load_driver(raw::gdrv_sys, sizeof(raw::gdrv_sys), L"C:\\Windows\\System32\\drivers\\gdrv.sys", L"gdrv"))
				return;
			global::logger->log("[info] driver loaded");
		}
		else
			global::logger->log("[info] driver already loaded");

		// get driver handle
		_handle = CreateFile((std::wstring(L"\\\\.\\") + DEVICE_NAME).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

		// check driver handle
		if (_handle == INVALID_HANDLE_VALUE)
		{
			global::logger->log("[error] driver handle invalid");
			return;
		}
		global::logger->log("[info] got handle to driver: %#08x", reinterpret_cast<std::uint64_t>(_handle));

		_init = true;
	}

	driver::~driver() noexcept
	{
		CloseHandle(_handle);
	}

	native::virt_addr driver::map_physical(native::phys_addr phys_addr, uint32_t size, bool unsafe)
	{
		if (size > PAGE_SIZE && !unsafe)
		{
			global::logger->log("[warn] tried to allocate more than one page");
			return 0;
		}
		if (_pages_mapped > 0 && !unsafe)
		{
			global::logger->log("[warn] one or more pages are already mapped");
			return 0;
		}

		MAP_PHYS_STRUCT in = { 0, 0, phys_addr, 0, size };
		native::virt_addr out = { 0 };

		::DeviceIoControl(_handle, IOCTL_MAP_PHYSICAL, &in, sizeof(in), &out, sizeof(out), nullptr, nullptr);
		++_pages_mapped;

		return out;
	}

	void driver::unmap_physical(native::virt_addr virt_addr)
	{
		uint64_t in = { virt_addr };
		::DeviceIoControl(_handle, IOCTL_UNMAP_PHYSICAL, &in, sizeof(in), nullptr, 0, nullptr, nullptr);
		--_pages_mapped;
	}

	void driver::memcpy(native::virt_addr dest, native::virt_addr src, uint32_t size) const
	{
		MEMCPY_STRUCT in = { dest, src, size };
		::DeviceIoControl(_handle, IOCTL_MEMCPY, &in, sizeof(in), nullptr, 0, nullptr, nullptr);
	}

	void driver::run_smartfan() const
	{
		::DeviceIoControl(_handle, IOCTL_SMARTFAN, nullptr, 0, nullptr, 0, nullptr, nullptr);
	}

	bool driver::is_init() const
	{
		return _init;
	}

	native::phys_addr driver::translate_linear_address(native::virt_addr dirbase, native::virt_addr virt_addr)
	{
		translation::VIRT_ADDR addr = { virt_addr };
		translation::PTE_CR3 cr3 = { dirbase };

		translation::PML4E pml4e = { read_physical_memory<uint64_t>(PFN_TO_PAGE(cr3.pml4_p) + sizeof(translation::PML4E) * addr.pml4_index) };
		if (!pml4e.present)
			return 0;

		translation::PDPTE pdpte = { read_physical_memory<uint64_t>(PFN_TO_PAGE(pml4e.pdpt_p) + sizeof(translation::PDPTE) * addr.pdpt_index) };
		if (!pdpte.present)
			return 0;

		translation::PDE pde = { read_physical_memory<uint64_t>(PFN_TO_PAGE(pdpte.pd_p) + sizeof(translation::PDE) * addr.pd_index) };
		if (!pde.present)
			return 0;

		translation::PTE pte = { read_physical_memory<uint64_t>(PFN_TO_PAGE(pde.pt_p) + sizeof(translation::PTE) * addr.pt_index) };
		if (!pte.present)
			return 0;

		return PFN_TO_PAGE(pte.page_frame) + addr.offset;
	}
}