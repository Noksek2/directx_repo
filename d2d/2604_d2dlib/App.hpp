
#include "d2d.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include <chrono>
#include <string>
static void tsprintf(TCHAR* pbuf, const TCHAR* fstr, ...) {
	va_list li;
	va_start(li, fstr);
	_vstprintf(pbuf, fstr, li);
	va_end(li);

}

class App {
public:
	const HWND GetHWND() { return m_hwnd; }
	D2D* const GetD2D() { return m_d2d; }
	~App() {

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		if (m_d2d) {
			delete m_d2d;
			m_d2d = nullptr;
		}
	}

	bool InitImGui() {
		auto d3d = m_d2d->mD3D;
		auto mDevice = d3d->mDevice;
		if (mDevice == nullptr) {
			return false;
		}
		// 1. ImGui 컨텍스트 생성
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 키보드 컨트롤 활성화
		// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // 도킹 활성화

		// 2. 스타일 설정
		ImGui::StyleColorsDark(); // (기본 다크 모드)

		// 3. 백엔드 초기화
		//    (반드시 윈도우와 DX11 장치가 먼저 생성되어야 함)
		ImGui_ImplWin32_Init(d3d->mHwnd);
		ImGui_ImplDX11_Init(mDevice, d3d->mDeviceContext);

		// 4. (선택) 폰트 로드
		// io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Malgun.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesKorean());
		return true;
	}
	void ImGui_BeginRender() {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	void ImGui_EndRender() {
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());//
	}
	virtual bool Create(HINSTANCE hin, WNDPROC proc, const TCHAR* className, const TCHAR* title, int screenWidth, int screenHeight) {

		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = proc;
		wc.hInstance = hin;
		wc.lpszClassName = className;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		RegisterClass(&wc);

		m_hwnd = CreateWindowExW(0, className, title, WS_OVERLAPPEDWINDOW,
								 CW_USEDEFAULT, CW_USEDEFAULT,
								 screenWidth, screenHeight, nullptr, nullptr, hin, nullptr);

		// D3D11 Device 생성
		D3D* d3d = new D3D();
		if (!d3d->Init(m_hwnd, screenWidth, screenHeight)) {
			MessageBox(m_hwnd, L"D3D Initialize Failed", L"Error", MB_ICONERROR);
			return false;
		}
		// D2D 초기화
		m_d2d = new D2D();
		if (!m_d2d->Init(d3d, m_hwnd, d3d->mDevice, d3d->mSwapChain)) {
			MessageBox(m_hwnd, L"D2D Initialize Failed", L"Error", MB_ICONERROR);
			return false;
		}
		if (!m_d2d->InitWICFactory()) {
			MessageBox(m_hwnd, L"WICFactory Failed", L"Error", MB_ICONERROR);
			return false;
		}
		if (!this->InitImGui()) {
			MessageBox(m_hwnd, _T("Failed to D3D Initialize, or do not initialize yet"), _T(""), MB_OK);
			return false;
		}


		return true;
		//if (!d3d->InitImGui()) {
		//	MessageBox(hwnd, _T("ImGui Error"), _T("ImGui Error"), MB_ICONERROR);
		//	return 0;
		//}
		//SetupImGuiStyle();
	}
	virtual void Render() = 0;
	virtual bool InitRender() {
		return true;
	}
	virtual void Show() {
		ShowWindow(m_hwnd, TRUE);
		UpdateWindow(m_hwnd);
	}
	virtual void ResizeScreen(const int W, const int H) { m_d2d->ResizeApp(m_hwnd, W, H); }
	void GetTick() {
		auto currentTime = std::chrono::steady_clock::now();
		// 마지막 프레임과의 시간 차이 (초 단위)
		float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastTime).count() / 1000000.0f;
		m_lastTime = currentTime;

		// 1초마다 FPS 갱신 (더 부드러운 표시)
		m_timeElapsed += deltaTime;
		m_frameCount++;
		if (m_timeElapsed >= 1.0f) {
			m_fps = static_cast<float>(m_frameCount) / m_timeElapsed;
			m_frameCount = 0;
			m_timeElapsed = 0.0f;

			TCHAR buf[64];
			tsprintf(buf, _T("My DX11 App (FPS: %.1f)"), m_fps);
			//fpsText += L", CNT : ";
			//fpsText += std::to_wstring(shapecounts) + L", CNT2 : ";
			//fpsText += std::to_wstring(shapecounts2) + L")";
			// m_hwnd는 WinApp 클래스에 있는 윈도우 핸들입니다.
			SetWindowTextW(m_hwnd, buf);

		}
	}
protected:
	D2D* m_d2d = nullptr;
	HWND m_hwnd;
	float m_fps = 0.0f;
	int m_frameCount = 0;
	float m_timeElapsed = 0.0f;
	std::chrono::steady_clock::time_point m_lastTime; // 마지막 프레임 시간

};