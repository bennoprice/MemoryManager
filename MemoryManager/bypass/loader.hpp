#pragma once
#include <Winternl.h>
#include <string_view>

namespace bypass
{
	namespace loader
	{
		extern "C" NTSTATUS NTAPI ZwLoadDriver(PUNICODE_STRING str);

		bool is_driver_loaded(std::string_view name);
		bool load_driver(const std::uint8_t* driver, std::size_t size, std::wstring_view path, std::wstring_view service);
		bool create_driver_service(const wchar_t* service_name, const wchar_t* driver_path, int startup_type);
		bool start_driver(const wchar_t* service_name);
		bool set_privilege(LPCTSTR privilege, bool enable);
	}
}