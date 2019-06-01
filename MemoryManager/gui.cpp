#include <iostream> 
#include "gui.hpp"
#include "config.hpp"
#include "menus/menus.hpp"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#pragma comment(lib, "d3d11.lib")

namespace global
{
	extern std::unique_ptr<gui::menu> menu;
	extern std::unique_ptr<menus::debug_log::logger> logger;
}

namespace ImGui
{
	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}

namespace gui
{
	LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return true;

		switch (msg)
		{
		case WM_SIZE:
			if (global::menu && global::menu->_p_device != nullptr && wparam != SIZE_MINIMIZED)
			{
				global::menu->cleanup_render_target();
				global::menu->_p_swapchain->ResizeBuffers(0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);
				global::menu->create_render_target();
			}
			return 0;
		case WM_SYSCOMMAND:
			if ((wparam & 0xfff0) == SC_KEYMENU)
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	menu::menu(std::wstring_view title)
	{
		// create window
		_wc = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0, 0, ::GetModuleHandle(0), 0, 0, 0, 0, L"window", 0 };
		::RegisterClassEx(&_wc);
		_hwnd = ::CreateWindow(_wc.lpszClassName, title.data(), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, 0, 0, _wc.hInstance, 0);

		// init direct3d
		if (!create_device(_hwnd))
		{
			global::logger->log("[error] failed to create device");
			cleanup_device();
			::UnregisterClass(_wc.lpszClassName, _wc.hInstance);
			return;
		}

		// show window
		::ShowWindow(_hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(_hwnd);

		// setup imgui
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowRounding = 0.f;
		style.WindowPadding = { 10.f, 9.f };
		style.ItemSpacing = { 8.f, 6.f };
		style.IndentSpacing = 40.f;

		style.Colors[ImGuiCol_TitleBg] = { 0.125f, 0.224f, 0.370f, 1.000f };
		style.Colors[ImGuiCol_Separator] = { 0.583f, 0.583f, 0.624f, 0.500f };
		style.Colors[ImGuiCol_SeparatorHovered] = { 0.652f, 0.652f, 0.691f, 0.500f };
		style.Colors[ImGuiCol_SeparatorActive] = { 0.698f, 0.698f, 0.735f, 0.500f };
		style.Colors[ImGuiCol_Border] = { 0.729f, 0.729f, 0.729f, 0.500f };

		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = NULL;

		ImGui_ImplWin32_Init(_hwnd);
		ImGui_ImplDX11_Init(_p_device, _p_device_context);

		global::logger->log("[info] initialised menu");
	}

	menu::~menu()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		cleanup_device();
		::DestroyWindow(_hwnd);
		::UnregisterClass(_wc.lpszClassName, _wc.hInstance);
	}

	bool menu::render(std::function<void()> action)
	{
		if (_msg.message == WM_QUIT)
			return false;

		if (::PeekMessage(&_msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&_msg);
			::DispatchMessage(&_msg);
			return true;
		}

		// start imgui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		action();

		// render
		ImGui::Render();
		_p_device_context->OMSetRenderTargets(1, &_p_render_target_view, 0);
		_p_device_context->ClearRenderTargetView(_p_render_target_view, (float*)&config::settings::background_colour);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		_p_swapchain->Present(1, 0);

		return true;
	}

	bool menu::create_device(HWND hwnd)
	{
		// setup swapchain
		DXGI_SWAP_CHAIN_DESC sd;
		::ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		UINT create_device_flags = 0;
		D3D_FEATURE_LEVEL feature_level;
		const D3D_FEATURE_LEVEL feature_level_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		if (D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &_p_swapchain, &_p_device, &feature_level, &_p_device_context) != S_OK)
			return false;

		create_render_target();
		return true;
	}

	void menu::cleanup_device()
	{
		cleanup_render_target();
		if (_p_swapchain) { _p_swapchain->Release(); _p_swapchain = 0; }
		if (_p_device_context) { _p_device_context->Release(); _p_device_context = 0; }
		if (_p_device) { _p_device->Release(); _p_device = 0; }
	}

	void menu::create_render_target()
	{
		ID3D11Texture2D* p_back_buffer;
		_p_swapchain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer));
		_p_device->CreateRenderTargetView(p_back_buffer, 0, &_p_render_target_view);
		p_back_buffer->Release();
	}

	void menu::cleanup_render_target()
	{
		if (_p_render_target_view) { _p_render_target_view->Release(); _p_render_target_view = 0; }
	}
}