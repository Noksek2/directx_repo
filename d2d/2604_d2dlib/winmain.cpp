
#include "App.hpp"

class MyApp : public App {
public:
	bool Create(HINSTANCE hin, WNDPROC proc, const TCHAR* className, const TCHAR* title, int screenWidth, int screenHeight) override {
		bool res = App::Create(hin, proc, className, title, screenWidth, screenHeight);
		if (!res) return false;

		HRESULT hr;

		auto mDC = m_d2d->mDeviceContext;
		hr = mDC->CreateSolidColorBrush(D2D1::ColorF(0.5, 0.1, 0.2, 0.6), &m_brush);
		if (FAILED(hr)) return false;
		return true;
	}
	bool InitRender() override {
		App::InitRender();
		if (!m_d2d->LoadWICImageFromFile(L"E:\\dxfrog_shit.jpg", &m_bitmap)) goto lError;
		if (!m_d2d->LoadWICImageFromFile(L"E:\\baka.bmp", &m_bitmap2)) goto lError;

		return true;
	lError:
		MessageBox(0, L"BITMAP LOAD ERROR", L"", MB_ICONERROR);
		return false;
	}
	void Render() override {
		auto mDC = m_d2d->mDeviceContext;
		const float clearColor[4] = { 0.1f,0.1f,0.1f,1.0f };

		ImGui_BeginRender();

		ImGui::Begin("Window B");
		ImGui::Text("This is the second window. DOD Style");
		static float rgba[4] = { 1.0f, 1.0f,1.0f,1.0f };

		if (ImGui::ColorEdit4("Color (RGBA)", rgba)) {
			m_brush->SetColor(D2D1::ColorF(rgba[0], rgba[1], rgba[2], rgba[3]));
		}
		ImGui::SliderFloat4("Bitmap Size", m_bitmap_pos, 0.f, 600.f);
		ImGui::End();


		D3D* d3d = m_d2d->mD3D;
		d3d->mDeviceContext->ClearRenderTargetView(d3d->mRtView, clearColor);
		m_d2d->BeginDraw();
		m_d2d->Clear(D2D1::ColorF(D2D1::ColorF::White));
		mDC->FillRectangle(D2D1::RectF(0, 0, 100, 100), m_brush.Get());

		mDC->FillRectangle(D2D1::RectF(50, 50, 150, 150), m_brush.Get());
		mDC->DrawBitmap(m_bitmap.Get(), D2D1::RectF(m_bitmap_pos[0], m_bitmap_pos[1], m_bitmap_pos[2], m_bitmap_pos[3]));
		mDC->DrawBitmap(m_bitmap2.Get(), D2D1::RectF(20, 20, 300, 300));
		m_d2d->EndDraw();
		d3d->mDeviceContext->OMSetRenderTargets(1, &d3d->mRtView, nullptr);

		ImGui_EndRender();
		d3d->mSwapChain->Present(1, 0);
	}
protected:
	ComPtr<ID2D1SolidColorBrush> m_brush;
	ComPtr<ID2D1Bitmap> m_bitmap;
	ComPtr<ID2D1Bitmap> m_bitmap2;
	float m_bitmap_pos[4] = { 0.f,0.f,0.f,0.f };
};

MyApp *g_app;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

	const int screenWidth = 800;
	const int screenHeight = 600;

	g_app = new MyApp();
	if (!g_app->Create(hInstance, WndProc, L"D2DClassName", L"D2D Title", screenWidth, screenHeight)) {
		//MessageBox(0,L"D2D Error")
		delete g_app;
		return 0;
	}

	g_app->InitRender();
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			g_app->GetTick();
			g_app->Render();
		}
	}
	delete g_app;
	return (int)msg.wParam;
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;

	switch (msg) {
	case WM_PAINT: {
		ValidateRect(hwnd, NULL);
		return 0;
	}
	case WM_SIZE:
		if (g_app && wParam != SIZE_MINIMIZED) {
			int newWidth = (int)LOWORD(lParam);
			int newHeight = (int)HIWORD(lParam);
			g_app->ResizeScreen(newWidth, newHeight);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}