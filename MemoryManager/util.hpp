#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <functional>

namespace util
{
	void iterate_processes(std::function<void(PROCESSENTRY32W proc)> action);
}