#include "App.h"

void MyWinApp::OnResize(UINT width, UINT height) {

	m_currentHeight = height;
	m_currentWidth = width;
	// --- 1. 모든 D3D/D2D 인터페이스 가져오기 ---
	auto& g_pSwapChain = GetD3D().m_swapChain;
	auto& g_pImmediateContext = GetD3D().m_deviceContext;
	auto& g_pRenderTargetView = GetD3D().m_backBufferView;
	auto& g_pDepthStencilView = GetD3D().m_depthStencilView; // Z-버퍼 뷰
	auto& g_pd3dDevice = GetD3D().m_device;
#ifdef USED2D
	auto* pD2DContext = m_d2d->GetDeviceContext(); // D2D 컨텍스트
#endif

	if (g_pSwapChain == nullptr) return; // 초기화 전이면 무시

	// --- 2. 리사이즈 전, 모든 타겟 해제 (매우 중요) ---
	g_pImmediateContext->OMSetRenderTargets(0, 0, 0);
	g_pRenderTargetView.Reset();
	g_pDepthStencilView.Reset(); // Z-버퍼도 반드시 해제
#ifdef USED2D
	if (pD2DContext) pD2DContext->SetTarget(nullptr);
#endif

	// --- 3. 스왑 체인(캔버스) 리사이즈 ---
	// (D2D를 쓰신다면 B8G8R8A8이 필요할 수 있습니다)
	HRESULT hr = g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
	if (FAILED(hr))
	{
		// 리사이즈 실패 (치명적 오류)
		return;
	}

	// --- 4. D3D 렌더 타겟 뷰 (RTV) 재생성 ---
	ComPtr<ID3D11Texture2D> pBackBuffer;
	hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (SUCCEEDED(hr))
	{
		hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, g_pRenderTargetView.GetAddressOf());
	}
	if (FAILED(hr)) return; // 실패 시 종료

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
		hr = g_pd3dDevice->CreateDepthStencilView(depthBufferTexture.Get(), &dsvDesc, g_pDepthStencilView.GetAddressOf());
	}
	if (FAILED(hr)) return; // 실패 시 종료

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
	hr = g_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&dxgiBackBuffer);
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