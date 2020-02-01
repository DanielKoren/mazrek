#include "src\visuals\window.hpp"
#include "src\visuals\graphics.hpp"

enum main_status : int
{
	err_success = 0x0,
	err_debug_priv = 1U << 0,//0x00000001
	err_window = 1U << 1,	//0x00000002
	err_graphics = 1U << 2	//0x00000004
};

int __stdcall WinMain
(
	_In_ HINSTANCE	instance,
	_In_opt_ HINSTANCE	prev_instance,
	_In_ LPSTR		cmd_args,
	_In_ int		cmd_show
)
{
	using namespace visuals;

	auto wnd = window(instance);
	if (!wnd.initialise())
	{
		return err_window;
	}

	auto gui = graphics(wnd);
	if (!gui.initialise())
	{
		return err_graphics;
	}	

	while (wnd.is_running())
	{
		if (wnd.message_loop())
			continue;

		gui.render();
	}

	return err_success;
}