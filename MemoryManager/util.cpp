#include "util.hpp"

namespace util
{
	void iterate_processes(std::function<void(PROCESSENTRY32W proc)> action)
	{
		auto snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		auto pe = PROCESSENTRY32W{ sizeof(PROCESSENTRY32W) };
		if (::Process32First(snapshot, &pe))
		{
			do
				action(pe);
			while (::Process32Next(snapshot, &pe));
		}
		::CloseHandle(snapshot);
	}
}