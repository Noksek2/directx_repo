#include "App.hpp"
WinApp* g_app;
WinApp::WinApp() {
	g_app = this;
	m_lastTime = std::chrono::steady_clock::now();
}
WinApp::~WinApp() {
	g_app = nullptr;
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
ATOM WinApp::RegisterApp(HINSTANCE hInstance, const TCHAR* wndname) {
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
bool WinApp::CreateMyWindow(HINSTANCE hin, const TCHAR* title, UINT width, UINT height) {
	if (!RegisterApp(hin, title)) {
		MessageBoxW(nullptr, L"FAiled to Register Window", L"Error", MB_OK);
		return false;
	}
	RECT rc = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindowEx(
		0UL, 
		title, 
		title, 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		rc.right - rc.left,
		rc.right - rc.left,
		nullptr, 
		nullptr, 
		hin, 
		nullptr
	);
	if (!m_hwnd) {
		MessageBoxW(nullptr, L"FAiled to Create Window", L"Error", MB_OK);
		return false;
	}
	return true;
}
bool WinApp::Create(HINSTANCE hin, const TCHAR* title, UINT width, UINT height)
{
	g_app = this;
	if (!CreateMyWindow(hin, title, width, height)) {
		return false;
	}
	if (!m_engine.Init(m_hwnd, width, height)) {
		return false;
	}
	m_running = true;
	return true;
}


void WinApp::OnResize(UINT width, UINT height) {

	m_currentHeight = height;
	m_currentWidth = width;
	// --- 1. 모든 D3D/D2D 인터페이스 가져오기 ---
	ComPtr<IDXGISwapChain> g_pSwapChain = GetD3D().GetSwapChain();
	ComPtr<ID3D11DeviceContext> g_pImmediateContext = GetD3D().GetDeviceContext();
	ComPtr<ID3D11RenderTargetView> g_pRenderTargetView = GetD3D().GetBackBufferView();
	ComPtr<ID3D11DepthStencilView> g_pDepthStencilView = GetD3D().GetDepthStencilView();
	ComPtr<ID3D11Device> g_pd3dDevice = GetD3D().GetDevice();
#ifdef USED2D
	ComPtr<ID2D1DeviceContext> pD2DContext = GetD2D().GetDeviceContext(); // D2D 컨텍스트
#endif

	//pD2DContext->SetTarget(nullptr);
	if (!g_pSwapChain) return; // 초기화 전이면 무시

	// --- 2. 리사이즈 전, 모든 타겟 해제 (매우 중요) ---
	g_pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);

	auto l = g_pRenderTargetView->Release();
	l = g_pDepthStencilView->Release();
#ifdef USED2D
	if (pD2DContext) 
		pD2DContext->SetTarget(nullptr);
#endif

	// --- 3. 스왑 체인(캔버스) 리사이즈 ---
	// (D2D를 쓰신다면 B8G8R8A8이 필요할 수 있습니다)
	HRESULT hr = g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
	if (FAILED(hr))
	{
		MessageBoxW(m_hwnd, L"SwapChain ResizeBuffers failed", L"Error", MB_OK);
		return;
	}

	// --- 4. D3D 렌더 타겟 뷰 (RTV) 재생성 ---
	ComPtr<ID3D11Texture2D> pBackBuffer;
	hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (SUCCEEDED(hr))
	{
		hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, g_pRenderTargetView.GetAddressOf());
	}
	if (FAILED(hr)) {

		MessageBoxW(nullptr, L"CreateRenderTargetView failed", L"Error", MB_OK);
		return;
	}

	// --- 5. D3D 깊이/스텐실 뷰 (DSV) 재생성 ---
	ComPtr<ID3D11Texture2D> depthBufferTexture;
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;  // 새 너비
	depthDesc.Height = height; // 새 높이
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Init과 동일한 포맷
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = g_pd3dDevice->CreateTexture2D(&depthDesc, nullptr, &depthBufferTexture);
	if (SUCCEEDED(hr))
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = depthDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		hr = g_pd3dDevice->CreateDepthStencilView(
			depthBufferTexture.Get(),
			&dsvDesc, 
			g_pDepthStencilView.ReleaseAndGetAddressOf());
	}
	if (FAILED(hr)) {
		MessageBoxW(nullptr, L"CreateDepthStencilView failed", L"Error", MB_OK);
		return;
	}


	// --- 6. 새 D3D 타겟 설정 ---
	g_pImmediateContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), g_pDepthStencilView.Get());


	// --- 7. (★핵심★) D3D 뷰포트 계산 (비율 유지) ---
	// D3D가 '쭉 늘어나는' 현상을 여기서 잡습니다.
	this->UpdateD3DViewport(width, height);


#ifdef USED2D
	// --- 8. D2D 타겟 재생성 (D3D 백버퍼와 연동) ---
	// D2D는 픽셀 좌표로 그리므로, 캔버스만 리사이즈되고 콘텐츠는 늘어나지 않습니다.
	// (7번의 D3D 비율 유지와 동일한 효과)

	ComPtr<IDXGISurface> dxgiBackBuffer;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)dxgiBackBuffer.GetAddressOf());
	if (SUCCEEDED(hr))
	{
		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED) // 포맷 확인
			);

		ComPtr<ID2D1Bitmap1> d2dTargetBitmap;
		hr = pD2DContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&d2dTargetBitmap
		);
		if (SUCCEEDED(hr))
		{
			pD2DContext->SetTarget(d2dTargetBitmap.Get());
		}
	}
#endif
}
bool WinApp::RenderInit() { return true; }
bool WinApp::Render() {

	//Time
	auto currentTime = std::chrono::steady_clock::now();
	// 마지막 프레임과의 시간 차이 (초 단위)
	float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastTime).count() / 1000000.0f;
	m_deltaTime = deltaTime;
	m_lastTime = currentTime;

	// 1초마다 FPS 갱신 (더 부드러운 표시)
	m_timeElapsed += deltaTime;
	m_frameCount++;
	if (m_timeElapsed >= 1.0f) {
		m_fps = static_cast<float>(m_frameCount) / m_timeElapsed;
		m_frameCount = 0;
		m_timeElapsed = 0.0f;

		// ★★★ 여기에 FPS를 출력하는 코드가 들어갑니다 ★★★
		// (아래 2. 텍스트 출력 방법 참고)
		std::wstring fpsText = L"My DX11 App (FPS: " + std::to_wstring(m_fps) + L")";
		// m_hwnd는 WinApp 클래스에 있는 윈도우 핸들입니다.
		SetWindowTextW(m_hwnd, fpsText.c_str());
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	this->Update();
	ImGui::Render(); // ImGui가 그릴 내용을 계산

	// ... (여기서 본인의 3D/2D 렌더링을 합니다) ...
	// ... (m_pImmediateContext->ClearRenderTargetView() 등) ...
	// ... (m_D2DDeviceContext->BeginDraw() ... EndDraw() 등) ...

	// 4. ImGui가 계산한 Draw Data를 DX11에 넘겨서 최종 렌더링
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
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
			m_engine.BeginFrame(0.f, 0.f, 0.f, 1.f);
			this->Render();
			m_engine.EndFrame();
			//D3D::GetInstance().GetDeviceContext()->ClearDepthStencilView(
			//	D3D::GetInstance().GetDepthStencilView().Get(), // (D3D.h에 추가한 뷰)
			//	D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			//	1.0f, // 깊이 값 (1.0 = 가장 멈)
			//	0     // 스텐실 값
			//);
		}
	}
	/*
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/
	return (int)msg.wParam;
}


LRESULT CALLBACK WinApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
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
		if (wParam != SIZE_MINIMIZED && g_app != nullptr)
			g_app->OnResize(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
	//WinApp* app = nullptr;
	//
	//if (message == WM_NCCREATE)
	//{
	//	LPVOID lp = ((CREATESTRUCT*)lParam)->lpCreateParams;
	//	app = (WinApp*)(lp);
	//
	//	SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(app));
	//	//app->m_hwnd = hwnd;
	//}
	//else app = (WinApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	//
	//if (app)
	//	return app->HandleMessage(message, wParam, lParam);
	//return DefWindowProc(hwnd, message, wParam, lParam);

}