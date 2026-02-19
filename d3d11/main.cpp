
#include "App.h"
int WINAPI WinMain(HINSTANCE hin, HINSTANCE, LPSTR, int) {
	//DPI Setting fuck you
	//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	auto title = _T("fuck you");
	std::unique_ptr<MyWinApp>app(new MyWinApp(hin, title, 800, 600));

	HWND hwnd = app->getHwnd();
	// 응용 프로그램 초기화를 수행합니다.
	if (!hwnd)return 0;


	/*while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);
	   // else render(d2d.get());
	}*/
	 


	return (int)app->MsgLoop();
}
WPARAM WinApp::MsgLoop() {
	MSG msg = { 0 };
	RenderInit();
	while (1) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			//Windows Message Process
			//if quit message
			if (msg.message == WM_QUIT) {
				break;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		{
			float color[4] = { 0.f,0.f,0.f, 1.0f };


			D3D::GetInstance().GetDeviceContext()->ClearRenderTargetView(
				D3D::GetInstance().GetBackBufferView(), color);
			
			D3D::GetInstance().GetDeviceContext()->ClearDepthStencilView(
				D3D::GetInstance().GetDepthStencilView().Get(), // (D3D.h에 추가한 뷰)
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				1.0f, // 깊이 값 (1.0 = 가장 멈)
				0     // 스텐실 값
			);
		}
		this->Render();

		//Swapchain Present
		GetD3D().GetSwapChain()->Present(1, 0);
	}
	/*
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/

	D3D::DeleteInstance();
	return (int)msg.wParam;
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

