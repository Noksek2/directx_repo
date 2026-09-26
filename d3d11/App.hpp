#pragma once
#include "GraphicEngine.hpp"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


extern LRESULT CALLBACK g_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
class WinApp;

extern WinApp* g_app;

class WinApp {
public:
	WinApp();
	~WinApp();

	bool Create(HINSTANCE hInstance, const TCHAR* title, UINT width, UINT height);
	void Show() {
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		UpdateWindow(m_hwnd);
	}
	
	inline D3D& GetD3D() { return m_engine.GetD3D(); }
	inline D2D& GetD2D() { return m_engine.GetD2D(); }
	HWND GetHwnd() { return m_hwnd; }

	void OnResize(UINT, UINT);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp)
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
	
	bool RenderInit();
	bool Render();

	virtual void Update(){}
protected:
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

	bool CreateMyWindow(HINSTANCE hInstance, const TCHAR* title, UINT width, UINT height);
	ATOM RegisterApp(HINSTANCE hInstance, const TCHAR* wndname);


	HWND m_hwnd = NULL;
	GraphicEngine m_engine;
	std::chrono::steady_clock::time_point m_lastTime;
	double m_deltaTime = 0.0;
	float m_fps = 0.f;
	float m_timeElapsed = 0.f;
	int m_frameCount = 0;

	bool m_running = false;


	UINT m_screenWidth = 0;//origin
	UINT m_screenHeight = 0;//origin

	UINT m_currentWidth = 0u;
	UINT m_currentHeight = 0u;

};


//class MyWinApp :public WinApp {
//
//public:
//	//virtual bool RenderInit();
//	virtual bool Render();
//	MyWinApp(UINT width, UINT height) {
//		
//		m_screenWidth = width;
//		m_screenHeight = height;
//
//		m_currentWidth = width;
//		m_currentHeight = height;
//
//		m_bWinRatioFixed = true;
//
//		//if (!D3D::GetInstance().InitSpriteFont()) {
//		//	MessageBox(m_hwnd, _T("DirectXTK Error"), _T("DirectXTK Error"), MB_ICONERROR);
//		//	return;
//		//}
//#ifdef USEIMGUI
//		//if (!D3D::GetInstance().InitImGui()) {
//		//	MessageBox(m_hwnd, _T("DirectXImGui Error"), _T("ImGui Error"), MB_ICONERROR);
//		//	return;
//		//}
//#endif
//	}
//	~MyWinApp() {
//#ifdef USED2D
//		SAFE_RELEASE(m_bitmap);
//#endif
//#ifdef USEIMGUI
//		ImGui_ImplDX11_Shutdown();
//		ImGui_ImplWin32_Shutdown();
//		ImGui::DestroyContext();
//#endif
//	}
//
//	bool Init(HINSTANCE hin, const TCHAR* wndname) {
//		if (!WinApp::Create(hin, wndname, m_screenWidth, m_screenHeight)) {
//			return false;
//		}
//		WinApp::Show();
//		return true;
//	}
//
//	
//
//	void OnPaint() {
//
//	}
//
//	void OnResize(UINT, UINT);
//
//	LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp) override {
//#ifdef USEIMGUI
//		if (ImGui_ImplWin32_WndProcHandler(m_hwnd, msg, wp, lp))
//			return true;
//#endif
//		switch (msg)
//		{
//		case WM_CREATE:
//		{
//
//			//MessageBox(m_hwnd, L"sfasdf", L"asdfasdf", MB_OK);
//		}return 0;
//		case WM_COMMAND:
//		{
//		}
//		return 0;
//		case WM_PAINT:
//		{
//			this->OnPaint();
//		}ValidateRect(m_hwnd, NULL);
//		return 0;
//		case WM_DISPLAYCHANGE:
//		{
//			InvalidateRect(m_hwnd, NULL, FALSE);
//		}
//		return 0;
//		case WM_LBUTTONDOWN:
//			//InvalidateRect(m_hwnd, NULL, FALSE);
//			return 0;
//		case WM_SIZE:
//		{
//			if (wp != SIZE_MINIMIZED)
//				this->OnResize(LOWORD(lp), HIWORD(lp));
//		}return 0;
//		}
//		return WinApp::HandleMessage(msg, wp, lp);
//	}
//protected:
//
//	ComPtr<ID3D11Buffer> m_vb;
//	ComPtr<ID3D11Buffer> m_ib;
//#ifdef USED2D
//	ID2D1Bitmap* m_bitmap;
//
//#endif	
//	DirectX::XMFLOAT2 m_pos;
//
//
//	int m_rotType;
//	float m_def;
//	float color;
//
//
//
//
//	bool m_bD2D = false;
//	bool m_bWinRatioFixed = false;
//
//public:
//	ComPtr<ID3D11ShaderResourceView> m_texture;
//	float f = 0.f;
//
//};
//