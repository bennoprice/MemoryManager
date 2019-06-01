#include <Windows.h>
#include <cstdint>
#include <fstream>
#include <psapi.h>
#include <string_view>
#include "loader.hpp"
#include "../menus/menus.hpp"

namespace global
{
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace bypass
{
	namespace loader
	{
		bool is_driver_loaded(std::string_view name)
		{
			void* drivers[1024] = { 0 };
			::EnumDeviceDrivers(drivers, sizeof(drivers), 0);

			for (int i = 0; drivers[i]; ++i)
			{
				char driver_name[MAX_PATH];
				::GetDeviceDriverBaseNameA(drivers[i], driver_name, MAX_PATH);

				if (!::_stricmp(driver_name, name.data()))
					return true;
			}
			return false;
		}

		bool load_driver(const std::uint8_t* driver, std::size_t size, std::wstring_view path, std::wstring_view service)
		{
			// dump driver to disk
			std::ofstream file(path.data(), std::ios::binary);
			file.write(reinterpret_cast<const char*>(driver), size);
			file.close();
			global::logger->log("[info] dumped driver to disk");

			// create service
			if (!create_driver_service(service.data(), path.data(), 3))
				return false;

			// start driver
			if (!start_driver(service.data()))
				return false;

			return true;
		}

		bool create_driver_service(const wchar_t* service_name, const wchar_t* driver_path, int startup_type)
		{
			HKEY key;
			HKEY subkey;

			DWORD type = 1;
			DWORD error_control = 1;

			wchar_t path[MAX_PATH];
			swprintf_s(path, ARRAYSIZE(path), L"\\??\\%s", driver_path);

			if (RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", &key))
			{
				global::logger->log("[error] failed to open registry");
				return false;
			}

			if (RegCreateKeyW(key, service_name, &subkey))
			{
				RegCloseKey(key);
				global::logger->log("[error] failed to create registry key");
				return false;
			}

			LSTATUS status = 0;
			status |= RegSetValueExW(subkey, L"ErrorControl", 0, REG_DWORD, (const BYTE*)&error_control, sizeof(error_control));
			status |= RegSetValueExW(subkey, L"ImagePath", 0, REG_EXPAND_SZ, (const BYTE *)path, (sizeof(WCHAR) * ((DWORD)wcslen(path) + 1)));
			status |= RegSetValueExW(subkey, L"Start", 0, REG_DWORD, (const BYTE *)&startup_type, sizeof(startup_type));
			status |= RegSetValueExW(subkey, L"Type", 0, REG_DWORD, (const BYTE*)&type, sizeof(type));

			RegCloseKey(subkey);

			if (status)
			{
				global::logger->log("[error] failed to set registry key values");
				return false;
			}

			global::logger->log("[info] created driver service");
			return true;
		}

		bool start_driver(const wchar_t* service_name)
		{
			UNICODE_STRING str;
			auto wpath = std::wstring(L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\") + service_name;
			RtlInitUnicodeString(&str, wpath.c_str());

			// give process driver loading privileges
			set_privilege(TEXT("SeLoadDriverPrivilege"), TRUE);

			// start driver
			if (ZwLoadDriver(&str) < 0)
			{
				global::logger->log("[error] failed to load driver");
				return false;
			}

			return true;
		}

		bool set_privilege(LPCTSTR privilege, bool enable)
		{
			HANDLE token;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
			{
				global::logger->log("[error] failed to open process token");
				return false;
			}

			TOKEN_PRIVILEGES tp;
			LUID luid;

			if (!LookupPrivilegeValue(NULL, privilege, &luid))
			{
				CloseHandle(token);
				global::logger->log("[error] failed to find privilege value");
				return false;
			}

			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			if (enable)
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			else
				tp.Privileges[0].Attributes = 0;

			// Enable the privilege or disable all privileges.
			if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
			{
				CloseHandle(token);
				global::logger->log("[error] failed to adjust token privilege");
				return false;
			}

			if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
			{
				CloseHandle(token);
				global::logger->log("[error] not all assigned");
				return false;
			}

			CloseHandle(token);
			return true;
		}
	}
}