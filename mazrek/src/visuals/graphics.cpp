#include "window.hpp"
#include "graphics.hpp"

#include "..\core\inject.hpp"
#include "..\core\logger.hpp"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_internal.h"

#include "font\pixeltype.h"
#include "img\logo.h"

#include "..\misc\helpers.hpp"

#include <algorithm>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define	IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define	IM_FLAGS	ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_ShowBorders

namespace visuals
{
	images_t graphics::m_images{};
	FILE* console_output_stream = nullptr;

	graphics::graphics(window& wnd)
		: m_wnd(wnd)
	{
	}

	graphics::~graphics()
	{
		ImGui_ImplDX9_Shutdown();

		if (m_direct3d_device)
			m_direct3d_device->Release();

		if (m_direct3d)
			m_direct3d->Release();
	}

	bool graphics::initialise()
	{
		if (!init_direct3d())
		{
			MessageBox(nullptr, "Failed to initialise Direct3D", "error", MB_ICONERROR | MB_OK);
			return false;
		}

		if (!ImGui_ImplDX9_Init(m_wnd.get_wnd_handle(), m_direct3d_device))
		{
			MessageBox(nullptr, "Failed to initialise ImGui", "error", MB_ICONERROR | MB_OK);
			return false;
		}

		auto& io = ImGui::GetIO();
		auto font_cfg_template = ImFontConfig();
		font_cfg_template.FontDataOwnedByAtlas = false;

		const float font_size = 10.0f;
		io.Fonts->AddFontFromMemoryTTF(font_pixeltype, sizeof(font_pixeltype), font_size, &font_cfg_template);

		auto style = &ImGui::GetStyle();
		set_imgui_style(style);

		return true;
	}

	void graphics::render()
	{
		ImGui_ImplDX9_NewFrame();

		static bool render = true;
		ImGui::Begin("", &render, ImVec2(m_wnd.get_width(), m_wnd.get_height()), 0.f, IM_FLAGS);

		//logo
		{
			ImGui::Image(m_direct3d_texture, ImVec2(260, 70));
		}
		//close button
		{
			ImGui::SameLine(275);
			ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "x");
			if (ImGui::IsItemHovered())
			{
				ImGui::SameLine(275);
				ImGui::TextColored(ImVec4(1.0f, 0.f, 0.f, 1.0f), "x");
				if (ImGui::IsItemClicked())
				{
					PostQuitMessage(0);
				}
			}
			ImGui::Spacing();
		}

		//
		render_main();
		render_options();
		//

		ImGui::End();

		//endscene
		m_direct3d_device->SetRenderState(D3DRS_ZENABLE, false);
		m_direct3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		m_direct3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		const ImVec4 m_bg_color = ImColor(35, 33, 38);
		auto clear_col_dx = D3DCOLOR_RGBA((int)(m_bg_color.x * 255.0f), (int)(m_bg_color.y * 255.0f), (int)(m_bg_color.z * 255.0f), (int)(m_bg_color.w * 255.0f));
		m_direct3d_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

		if (m_direct3d_device->BeginScene() >= 0)
		{
			ImGui::Render();
			m_direct3d_device->EndScene();
		}

		m_direct3d_device->Present(NULL, NULL, NULL, NULL);
	}

	bool graphics::init_direct3d()
	{
		if ((m_direct3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		{
			return false;
		}

		ZeroMemory(&m_direct3d_pp, sizeof(m_direct3d_pp));
		m_direct3d_pp.Windowed = TRUE;
		m_direct3d_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		m_direct3d_pp.BackBufferFormat = D3DFMT_UNKNOWN;
		m_direct3d_pp.EnableAutoDepthStencil = TRUE;
		m_direct3d_pp.AutoDepthStencilFormat = D3DFMT_D16;
		m_direct3d_pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		if (m_direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_wnd.get_wnd_handle(),
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_direct3d_pp, &m_direct3d_device) < 0)
		{
			return false;
		}

		const int width = 260;
		const int height = 70;
		auto texture_status = D3DXCreateTextureFromFileInMemoryEx(m_direct3d_device, mazrek_logo_bytes, sizeof(mazrek_logo_bytes),
			width, height, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE,
			D3DCOLOR_ARGB(150, 100, 100, 100), NULL, NULL, &m_direct3d_texture);
		if (texture_status != D3D_OK)
		{
			return false;
		}

		m_direct3d_device->SetTexture(0, m_direct3d_texture);

		return true;
	}

	void graphics::set_imgui_style(ImGuiStyle* style)
	{
		style->WindowPadding = ImVec2(25, 25);
		style->WindowRounding = 0.f;
		style->FramePadding = ImVec2(7, 7);
		style->FrameRounding = 0.f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;

		style->Colors[ImGuiCol_Text] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 0.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.33f, 0.55f);
		style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.14f, 0.17f, 0.80f); //textbox e.g.
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
		style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 0.40f);
		style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.19f, 0.30f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.07f, 0.07f, 0.10f, 1.00f);
		style->Colors[ImGuiCol_Header] = ImVec4(0.07f, 0.07f, 0.10f, 1.00f);
		style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
		style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 0.2f, 0.5f, 0.4f);
		style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
	}

	void graphics::render_main()
	{
		if (!m_main_window)
			return;

		/*process selection*/
		{
			ImGui::PushItemWidth(-1);
			ImGui::ComboVec("", &m_current_process, m_processes, m_processes.size());
			//enumerate processes when using combo.
			if (ImGui::IsItemClicked())
			{
				m_processes.clear();
				auto processes = misc::enumerate_processes();

				//sort the vector alphabetically regardless the letter case
				std::sort(processes.begin(), processes.end(), 
					[](const std::pair<std::string, DWORD> first, const std::pair<std::string, DWORD> second)
					{
						uint32_t i = 0;

						while ((i < first.first.length()) && (i < second.first.length()))
						{
							if (tolower(first.first[i]) < tolower(second.first[i])) 
								return true;
							else if (tolower(first.first[i]) > tolower(second.first[i])) 
								return false;
							i++;
						}

						if (first.first.length() < second.first.length()) 
							return true;
						else
							return false;
					});

				for (const auto& process : processes) 
				{
					m_processes.emplace_back(process);
				}
			}
		}

		/*tooltip- reveal process id*/
		if (ImGui::IsItemHovered())
		{
			if (m_current_process)
			{
				auto process_id = m_processes.at(m_current_process).second;
				ImGui::SetTooltip("process id: %d", process_id);
			}
		}

		/*images listbox*/
		{
			ImGui::Text(" Images: ");
			ImGui::PushItemWidth(235);
			ImGui::ListBoxVec("##images", &m_current_image, m_images, m_images.size(), 4);
			ImGui::SameLine();

			auto images_dropdown_btn = ImGui::BeginButtonDropDown("##imagesopts", ImVec2(30, 80));

			if (images_dropdown_btn)
			{
				auto add_image = ImGui::Button("add", ImVec2(60, 24));
				auto remove_image = ImGui::Button("remove", ImVec2(60, 24));

				if (add_image)
				{
					const auto image_path = misc::open_filename(m_wnd.get_wnd_handle());
					if (!image_path.empty())
					{
						const auto image_name = misc::strip_path(image_path);

						bool image_exist = std::any_of(m_images.begin(), m_images.end(),
							[&image_path](const std::pair<std::string, std::string>& p)
							{
								return p.first == image_path;
							});

						if (!image_exist)
						{
							m_images.emplace_back(std::make_pair(image_path, image_name));
						}
					}
				}
				if (remove_image)
				{
					if (!m_images.empty())
					{
						m_images.erase(m_images.begin() + m_current_image);
						m_current_image -= m_current_image; //update images index
					}
				}

				ImGui::EndButtonDropDown();
			}
		}

		/*bottom options*/
		auto options_button = ImGui::Button("advanced options", ImVec2(130, 26)); 
		ImGui::SameLine(154);
		auto load_button = ImGui::Button("load image", ImVec2(130, 26));

		if (options_button)
		{
			m_main_window = !m_main_window;
			m_options_window = !m_options_window;
		}

		if (load_button && !m_images.empty() && m_processes.size() > 5)
		{
			const auto process_name = m_processes.at(m_current_process).first;
			const auto process_id = m_processes.at(m_current_process).second;
			const auto image_path = m_images.at(m_current_image).first;

			auto status = core::inject(process_id, image_path); //printf("%s - %d\nimage path: %s\n", process_name.c_str(), process_id, image_path.c_str());

			m_main_window = !m_main_window;
			m_options_window = !m_options_window;
		}
	}

	void graphics::render_options()
	{
		if (!m_options_window)
			return;

		/*injection options*/
		const char* injection_items[] = { "LoadLibraryA", "ManualMap" };
		const char* execution_items[] = { "CreateRemoteThread", "NtCreateThreadEx", "SetWindowHookEx", "Hijack Thread" };

		ImGui::Text("injection type");
		ImGui::SameLine(154);
		ImGui::Text("exection type");

		ImGui::PushItemWidth(130);
		ImGui::Combo("##injection", (int*)&core::options::inject, injection_items, IM_ARRAYSIZE(injection_items)); 
		ImGui::SameLine(154);
		ImGui::PushItemWidth(130);		
		ImGui::Combo("##execution", (int*)&core::options::execute, execution_items, IM_ARRAYSIZE(execution_items));

		//static bool console_option = false;
		//ImGui::Checkbox("enable console", &console_option);

		//static bool call_once = false;
		//if (console_option)
		//{
		//	if (!call_once)
		//	{
		//		AllocConsole();
		//		freopen_s(&console_output_stream, "conout$", "w", stdout);
		//		call_once = true;
		//	}
		//}
		//if (!console_option)
		//{
		//	if (call_once)
		//	{
		//		fclose(console_output_stream);
		//		FreeConsole();
		//		call_once = false;
		//	}
		//}

		//ImGui::SameLine();
		//
		//static bool v = false;
		//ImGui::Checkbox("load kernel driver", &v);

		/*logger window*/
		ImGui::Text("logs: ");
		ImGui::BeginChild("##logswindow", ImVec2(-1, 62), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (auto it = core::logger::logs.begin(); it != core::logger::logs.end(); ++it)
		{
			auto v_temp = *it;
			auto log_color = ImVec4(0.f, 0.f, 0.f, 255.f);

			switch (v_temp.second)
			{
			case core::logger::log_type::success_type:
				ImGui::Text("[ + ]");
				log_color = ImVec4(0.f, 130.f, 0.f, 255.f);
				break;
			case core::logger::log_type::information_type:
				ImGui::Text("[ - ]");
				log_color = ImVec4(225.f, 225.f, 225.f, 255.f);
				break;
			case core::logger::log_type::error_type:
				ImGui::Text("[ ! ]");
				log_color = ImVec4(130.f, 0.f, 0.f, 255.f);
				break;
			}

			ImGui::SameLine();
			ImGui::TextColored(log_color, v_temp.first.c_str());
		}
		ImGui::EndChild();

		auto back_button = ImGui::Button("<< back", ImVec2(-1, 26));
		if (back_button)
		{
			m_options_window = !m_options_window;
			m_main_window = !m_main_window;
		}
	}
}