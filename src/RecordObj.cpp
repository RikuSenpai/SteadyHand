#include "RecordObj.h"

std::ostream& operator<<(std::ostream &os, const MouseData &m_data) {
	os << m_data.ms_time << " " << m_data.dx << " " << m_data.dy 
		<< " " << m_data.mleft_code << std::endl;
	return os;
}

void MouseRecorderObj::RegisterInputDevices(HWND window) {
	// code referenced from Microsoft MSDN documentation:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms645546(v=vs.85).aspx#example_2
	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = kDesktopUsage;
	Rid[0].usUsage = kMouseUsage;
	Rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	Rid[0].hwndTarget = window;

	Rid[1].usUsagePage = kDesktopUsage;
	Rid[1].usUsage = kKeyBoardUsage;
	Rid[1].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	Rid[1].hwndTarget = window;

	if (!RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]))) {
		std::cout << "Device registration failed." << std::endl;
		return;
	}
	// code derived from:
	// https://github.com/rspeele/MouseMeat/blob/0b5b7aea844f7ab4158b4ebd34e8c2d1e702f995/Input.cpp#L227
	base_wnd_proc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
	std::cout << "Device registration successful." << std::endl;
}

bool MouseRecorderObj::CreateHiddenWindow(HINSTANCE &h_instance, HWND &window_handle) {
	WNDCLASS hid_wind_class;

	// code referenced from Microsoft MSDN documentation:
	// https://msdn.microsoft.com/en-us/library/ms633576(v=vs.85).aspx
	hid_wind_class.style = 0;
	hid_wind_class.lpfnWndProc = DefWindowProc;
	hid_wind_class.cbClsExtra = 0;
	hid_wind_class.cbWndExtra = 0;
	hid_wind_class.hInstance = h_instance;
	hid_wind_class.hIcon = NULL;
	hid_wind_class.hCursor = NULL;
	hid_wind_class.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
	hid_wind_class.lpszMenuName = NULL;
	hid_wind_class.lpszClassName = kClassName;

	if (RegisterClass(&hid_wind_class)) {
		window_handle = CreateWindow(kClassName, kClassName, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, h_instance, NULL);
		std::cout << "Window registration successful." << std::endl;
		return true;		
	}
	std::cout << "Registration of the window class failed." << std::endl;
	return false;
}

void MouseRecorderObj::WriteBufferToFile() {
	std::string filename = kWeaponNames[curr_weapon] + ".txt";
	std::ofstream pattern_file(filename);

	// booleans for keeping track of whether to save the mouse data.
	bool to_save_curr = false;
	for (MouseData m_data : mouse_data_list) {
		switch(m_data.mleft_code) {
		case 1:
			to_save_curr = true;
			break;
		case 2:
			to_save_curr = false;
			break;
		default:
			break;
		}

		if (to_save_curr) {
			pattern_file << m_data;
		}
	}
	pattern_file.close();
}

void MouseRecorderObj::RunMouseRecorder(HINSTANCE &h_instance, bool &is_recording) {
	HWND window_handle; // may change this to a member variable?
	CreateHiddenWindow(h_instance, window_handle);
	RegisterInputDevices(window_handle);
	std::cout << "setup complete." << std::endl;

	MSG message;
	BOOL msg_code;

	// The MS MSDN-recommended way to do message loops:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms644928(v=vs.85).aspx#creating_loop
	while (((msg_code = GetMessage(&message, NULL, 0, 0)) != 0) && is_recording) {
		if (msg_code == -1) {
			return;
		} else {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
}