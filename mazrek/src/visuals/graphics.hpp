#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3dx9tex.h>
#include "imgui\imgui.h"

namespace visuals
{
	using processes_t = std::vector<std::pair<std::string, DWORD>>; //process_name, process_id
	using images_t = std::vector<std::pair<std::string, std::string>>; //image_path, image_name

	class graphics
	{
	public:
		graphics(window& wnd);
		~graphics();

		bool initialise();
		void render();

		static images_t m_images; //for WM_DROPFILES
		int m_current_image = 0;

	private:
		window m_wnd;

		LPDIRECT3D9				m_direct3d = nullptr;
		D3DPRESENT_PARAMETERS	m_direct3d_pp{};
		LPDIRECT3DDEVICE9		m_direct3d_device = nullptr;
		LPDIRECT3DTEXTURE9		m_direct3d_texture = nullptr;

		bool init_direct3d();
		void set_imgui_style(ImGuiStyle* style);

		//
		processes_t m_processes{ std::make_pair("process selection", 0) };
		int m_current_process = 0;

		void render_main();
		bool m_main_window = true;
		void render_options();
		bool m_options_window = false;

	};
}
