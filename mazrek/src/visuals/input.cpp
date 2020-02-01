#include "input.hpp"
#include "window.hpp"
#include "graphics.hpp"
#include "..\misc\helpers.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"

#include <algorithm>

namespace visuals
{
	LRESULT CALLBACK window_procedure
	(
		_In_ HWND   hwnd,
		_In_ UINT   msg,
		_In_ WPARAM wparam,
		_In_ LPARAM lparam
	)
	{
		auto& io = ImGui::GetIO();
		static bool draggable = false;

		switch (msg)
		{
		case WM_LBUTTONDOWN:
			io.MouseDown[0] = true;
			SetCapture(hwnd);
			return true;
		case WM_LBUTTONUP:
			io.MouseDown[0] = false;
			ReleaseCapture();
			return true;
		case WM_RBUTTONDOWN:
			io.MouseDown[1] = true;
			return true;
		case WM_RBUTTONUP:
			io.MouseDown[1] = false;
			return true;
		case WM_MBUTTONDOWN:
			io.MouseDown[2] = true;
			return true;
		case WM_MBUTTONUP:
			io.MouseDown[2] = false;
			return true;
			//dragging window only at logo region
		case WM_MOUSEMOVE:
			io.MousePos.x = (signed short)(lparam);
			io.MousePos.y = (signed short)(lparam >> 16);
			if (io.MousePos.x < 250 && io.MousePos.y < 90) draggable = true;
			else draggable = false;
			return true;
		case WM_KEYDOWN:
			if (wparam < 256)
				io.KeysDown[wparam] = 1;
			return true;
		case WM_KEYUP:
			if (wparam < 256)
				io.KeysDown[wparam] = 0;
			return true;
		case WM_CHAR:
			if (wparam > 0 && wparam < 0x10000)
				io.AddInputCharacter((unsigned short)wparam);
			return true;
		case WM_NCHITTEST:
			if (DefWindowProc(hwnd, msg, wparam, lparam) == HTCLIENT && draggable && GetAsyncKeyState(MK_LBUTTON) < 0)
				return HTCAPTION;
			return DefWindowProc(hwnd, msg, wparam, lparam);

		case WM_CREATE:
			DragAcceptFiles(hwnd, TRUE);
			break;
		case WM_DROPFILES:
		{
			char path[MAX_PATH]{};
			UINT status = 0;
			HDROP drop_handle = (HDROP)wparam;

			if (DragQueryFile(drop_handle, 0xFFFFFFFF, NULL, NULL) != 1) 
			{
				MessageBox(hwnd, "Dropping multiple files is not supported", "error", MB_ICONERROR | MB_OK);
				DragFinish(drop_handle);
				break;
			}
			path[0] = '\0';
			if (DragQueryFile(drop_handle, 0, path, MAX_PATH)) 
			{
				if (!misc::file_exist(path))
					return MessageBox(hwnd, "Path is not valid", "error", MB_ICONERROR | MB_OK);

				const auto path_len = strlen(path);
				const auto extension = &path[path_len - 3];
				if (strcmp(extension, "dll"))
					return MessageBox(hwnd, "Only DLL files are allowed", "error", MB_ICONERROR | MB_OK);

				const auto file_name = misc::strip_path(path);

				bool image_exist = std::any_of(graphics::m_images.begin(), graphics::m_images.end(),
					[&path](const std::pair<std::string, std::string>& p)
					{
						return p.first == path;
					});

				if (!image_exist) 
				{
					graphics::m_images.emplace_back(std::make_pair(path, file_name));
				}
			}
			DragFinish(drop_handle);
			break;
		}
		case WM_SIZE:
			/* todo; unnecessary since the window is unresizable
			if (window::m_direct3d_device != NULL && wparam != SIZE_MINIMIZED) {
				ImGui_ImplDX9_InvalidateDeviceObjects();
				window::m_direct3d_pp.BackBufferWidth = LOWORD(lparam);
				window::m_direct3d_pp.BackBufferHeight = HIWORD(lparam);
				HRESULT hr = window::m_direct3d_device->Reset(&window::m_direct3d_pp);
				if (hr == D3DERR_INVALIDCALL)
					IM_ASSERT(0);
				ImGui_ImplDX9_CreateDeviceObjects();
			}*/
			return 0;
		case WM_SYSCOMMAND:
			if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			DragAcceptFiles(hwnd, FALSE);
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}