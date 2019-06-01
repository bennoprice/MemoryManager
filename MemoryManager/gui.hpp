#pragma once
#include <d3d11.h>
#include <functional>
#include <string_view>
#include "imgui/imgui.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

namespace gui
{
	class menu
	{
	public:
		explicit menu(std::wstring_view title);
		~menu();
		bool render(std::function<void()> action);
	private:
		bool create_device(HWND hwnd);
		void cleanup_device();
		void create_render_target();
		void cleanup_render_target();
		friend LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		ID3D11Device* _p_device = nullptr;
		ID3D11DeviceContext* _p_device_context = nullptr;
		IDXGISwapChain* _p_swapchain = nullptr;
		ID3D11RenderTargetView* _p_render_target_view = nullptr;

		WNDCLASSEX _wc = { 0 };
		HWND _hwnd = { 0 };
		MSG _msg = { 0 };
	};
}