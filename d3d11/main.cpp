
#include "App.hpp"
class MyApp:public WinApp {
public:
	void Update() override {
		auto rt = GetD2D().GetRenderTarget();
		ComPtr<ID2D1SolidColorBrush> brush;
		rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush);
		D2D1_RECT_F rect = D2D1::RectF(100.0f, 100.0f, 300.0f, 300.0f);
		rt->FillRectangle(&rect, brush.Get());
	}
};
int WINAPI WinMain(HINSTANCE hin, HINSTANCE, LPSTR, int) {
	//DPI Setting fuck you
	//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	auto title = _T("fuck you");
	std::unique_ptr<MyApp>app(new MyApp());
	if (!app->Create(hin, title, 800, 600)) return 1;
	app->Show();
	/*while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);
	   // else render(d2d.get());
	}*/


	return (int)app->MsgLoop();
}


LRESULT CALLBACK g_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {
	case WM_CREATE:
		return 0;

	case WM_COMMAND:
		return 0;
	
	case WM_PAINT:
	{/*
		PAINTSTRUCT ps;
		MessageBox(hwnd, L"", L"", MB_OK);
		HDC hdc = BeginPaint(hwnd, &ps);
		Ellipse(hdc, 0, 0, 100, 100);
		TextOut(hdc, 0, 50, _T("여기 출력되면 안되는데?"), 100);

		EndPaint(hwnd, &ps);*/
		ValidateRect(hwnd, 0);
	}
		return 0;
	case WM_DISPLAYCHANGE:
	{
	}
	return 0;
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		//InvalidateRect(m_hwnd, NULL, FALSE);
		return 0;
	case WM_SIZE:
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

