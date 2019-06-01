#pragma once
#include <Windows.h>
#include <cinttypes>
#include <memory>
#include "native.hpp"

namespace bypass
{
	constexpr auto DEVICE_NAME			= L"GIO";
	constexpr auto DRIVER_NAME			= "gdrv.sys";
	constexpr auto IOCTL_MAP_PHYSICAL	= 0xC3502004;
	constexpr auto IOCTL_UNMAP_PHYSICAL = 0xC3502008;
	constexpr auto IOCTL_MEMCPY			= 0xC3502808;
	constexpr auto IOCTL_SMARTFAN		= 0xC3500E68;


	typedef struct _MAP_PHYS_STRUCT
	{
		std::uint32_t interface_type;
		std::uint32_t bus;
		std::uint64_t physical_address;
		std::uint32_t io_space;
		std::uint32_t size;
	} MAP_PHYS_STRUCT, *PMAP_PHYS_STRUCT;

	typedef struct _MEMCPY_STRUCT
	{
		std::uint64_t dest;
		std::uint64_t src;
		std::uint32_t size;
	} MEMCPY_STRUCT, *PMEMCPY_STRUCT;

	class driver
	{
	public:
		explicit driver();
		~driver() noexcept;

		native::virt_addr map_physical(native::phys_addr phys_addr, std::uint32_t size, bool unsafe = false);
		native::phys_addr translate_linear_address(native::virt_addr dirbase, native::virt_addr virt_addr);
		void unmap_physical(native::virt_addr virt_addr);
		void memcpy(native::virt_addr dest, native::virt_addr src, uint32_t size) const;
		void run_smartfan() const;
		bool is_init() const;

		template<typename T>
		T read_physical_memory(native::phys_addr phys_addr)
		{
			auto virt_addr = map_physical(phys_addr, sizeof(T));
			auto buf = *reinterpret_cast<T*>(virt_addr);
			unmap_physical(virt_addr);
			return buf;
		}

		template<typename T>
		void write_physical_memory(native::phys_addr phys_addr, T val)
		{
			auto virt_addr = map_physical(phys_addr, sizeof(T));
			*reinterpret_cast<T*>(virt_addr) = val;
			unmap_physical(virt_addr);
		}

		template<typename T>
		T read_virtual_memory(native::virt_addr dirbase, native::virt_addr virt_addr)
		{
			auto phys_addr = translate_linear_address(dirbase, virt_addr);
			return read_physical_memory<T>(phys_addr);
		}

		template<typename T>
		void write_virtual_memory(native::virt_addr dirbase, native::virt_addr virt_addr, T val)
		{
			auto phys_addr = translate_linear_address(dirbase, virt_addr);
			write_physical_memory<T>(phys_addr, val);
		}

		template<typename T>
		T read_system_memory(native::virt_addr virt_addr) const
		{
			T buf;
			memcpy(reinterpret_cast<std::uint64_t>(&buf), virt_addr, sizeof(T));
			return buf;
		}

		template<typename T>
		void read_system_memory(native::virt_addr virt_addr, T val) const
		{
			auto buf = val;
			memcpy(virt_addr, reinterpret_cast<std::uint64_t>(&buf), sizeof(T));
		}
	private:
		HANDLE _handle;
		int _pages_mapped = 0;
		bool _init = false;
	};
}