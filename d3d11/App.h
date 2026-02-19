#pragma once
#define __APP2__
#include <chrono>
#include <string>
#include "D3D.h"

#ifdef USEIMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

extern LRESULT CALLBACK g_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

class WinApp {
protected:
	HWND m_hwnd;
	std::chrono::steady_clock::time_point m_lastTime;
	double m_deltaTime;
	float m_fps = 0.f;
	float m_timeElapsed = 0.f;
	int m_frameCount = 0;
	

	//std::unique_ptr<D3D> m_d3d;

	/*for FPS*/



	bool Create(HINSTANCE hInstance, const TCHAR* wndname, const TCHAR* title, HWND& hwnd, UINT width, UINT height, int nCmdShow = true)
	{
		hwnd = CreateWindowExW(0UL, wndname, title, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			nullptr, nullptr, hInstance, this);

		m_hwnd = hwnd;
		return hwnd;
	}
	ATOM RegisterApp(HINSTANCE hInstance, const TCHAR* wndname) {
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = this->WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, IDC_ARROW);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = wndname;
		wcex.lpszClassName = wndname;
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDC_ARROW);
		return RegisterClassExW(&wcex);
	}
	static inline D3D& GetD3D() {
		return D3D::GetInstance();
	}
public:

	HWND getHwnd() { return m_hwnd; }
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		WinApp* app = nullptr;

		if (message == WM_NCCREATE)
		{
			LPVOID lp = ((CREATESTRUCT*)lParam)->lpCreateParams;
			app = (WinApp*)(lp);

			SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(app));
			app->m_hwnd = hwnd;
		}
		else app = (WinApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (app)
			return app->HandleMessage(message, wParam, lParam);
		return DefWindowProc(hwnd, message, wParam, lParam);

	}
	virtual LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			MessageBox(m_hwnd, L"FUCK", L"FUCK", MB_OK);
			HDC hdc = BeginPaint(m_hwnd, &ps);
			Ellipse(hdc, 0, 0, 100, 100);
			TextOut(hdc, 0, 50, _T("여기 출력되면 안되는데?"), 100);

			EndPaint(m_hwnd, &ps);
		}
					 return 0;
		default:
			return DefWindowProc(m_hwnd, msg, wp, lp);
		}
	}
	WPARAM MsgLoop();
public:
	WinApp() {
		// initialize m_lastTime (current clock)
		m_lastTime = std::chrono::steady_clock::now();
	}
	virtual bool RenderInit();
	virtual bool Render();
};
class MyWinApp :public WinApp {
protected:
	bool m_bD2D = false;
	bool m_bInit = false;
	bool m_bWinRatioFixed = false;
	

	UINT m_screenWidth;//origin
	UINT m_screenHeight;//origin

	UINT m_currentWidth;
	UINT m_currentHeight;


	int m_rotType;
	float m_def;
	float color;
	DirectX::XMFLOAT2 m_pos;
	ComPtr<ID3D11Buffer> m_vb;
	ComPtr<ID3D11Buffer> m_ib;

#ifdef USED2D
	std::unique_ptr<D2D> m_d2d;
	ID2D1Bitmap* m_bitmap;

#endif

public:
	float f = 0.f;
	virtual bool RenderInit();
	virtual bool Render();
	ComPtr<ID3D11ShaderResourceView> m_texture;
public:

	MyWinApp(HINSTANCE hin, const TCHAR* wndname, UINT width, UINT height) {
		m_screenWidth = width;
		m_screenHeight = height;

		m_currentWidth = width;
		m_currentHeight = height;

		m_bWinRatioFixed = true;
		m_bInit = false;
		this->RegisterApp(hin, wndname);
		if (!this->Create(hin, wndname, wndname, m_hwnd, width, height))
			return;

		D3D::CreateInstance();
		D3D::GetInstance().Init(m_hwnd, width, height);
#ifdef USED2D
		m_d2d = std::make_unique<D2D>();
		m_d2d->Init(m_hwnd, D3D::GetInstance().m_device);
		m_d2d->InitWICFactory();
#else
		if (!D3D::GetInstance().InitSpriteFont()) {
			MessageBox(m_hwnd, _T("DirectXTK Error"), _T("DirectXTK Error"), MB_ICONERROR);
			return;
		}
#endif
#ifdef USEIMGUI
		if (!D3D::GetInstance().InitImGui()) {
			MessageBox(m_hwnd, _T("DirectXImGui Error"), _T("ImGui Error"), MB_ICONERROR);
			return;
		}
#endif
		m_bInit = true;
		ShowWindow(m_hwnd, true);
		UpdateWindow(m_hwnd);

	}
	~MyWinApp() {
#ifdef USED2D
		SAFE_RELEASE(m_bitmap);
#endif
#ifdef USEIMGUI
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif
	}

	void UpdateD3DViewport(UINT width, UINT height)
	{
		auto g_pImmediateContext = GetD3D().GetDeviceContext();
		D3D11_VIEWPORT vp = {};
		/*
				if (this->m_bWinRatioFixed)
				{
					// [2. 고정 모드] 뷰포트를 원본 크기로 고정 (여백 생김)
					vp.Width = (FLOAT)this->m_screenWidth;
					vp.Height = (FLOAT)this->m_screenHeight;
					vp.TopLeftX = 0.0f;
					vp.TopLeftY = 0.0f;
				}
				else
				{
		*/
		// [1. 스케일 모드] 뷰포트를 윈도우 전체로 설정 (쭉 늘어남)
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		//		}

		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		g_pImmediateContext->RSSetViewports(1, &vp);
	}


	void OnPaint() {

	}

	void OnResize(UINT, UINT);

	LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp) override {
#ifdef USEIMGUI
		if (ImGui_ImplWin32_WndProcHandler(m_hwnd, msg, wp, lp))
			return true;
#endif
		switch (msg)
		{
		case WM_CREATE:
		{

			//MessageBox(m_hwnd, L"sfasdf", L"asdfasdf", MB_OK);
		}return 0;
		case WM_COMMAND:
		{
		}
		return 0;
		case WM_PAINT:
		{
			this->OnPaint();
		}ValidateRect(m_hwnd, NULL);
		return 0;
		case WM_DISPLAYCHANGE:
		{
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		return 0;
		case WM_LBUTTONDOWN:
			//InvalidateRect(m_hwnd, NULL, FALSE);
			return 0;
		case WM_SIZE:
		{
			if (wp != SIZE_MINIMIZED)
				this->OnResize(LOWORD(lp), HIWORD(lp));
		}return 0;
		}
		return WinApp::HandleMessage(msg, wp, lp);
	}
};
