#include "D2D.hpp"
bool D2D::CreateD2DFactory() {
	D2D1_FACTORY_OPTIONS d2dOptions = {};

#ifdef _DEBUG
	d2dOptions.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
#endif
	HRESULT hr;
	hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) return false;

	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		//D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory1),
		&d2dOptions,
		(void**)&m_factory);
	if (FAILED(hr)) return false;
	return true;
}
bool D2D::CreateDWriteFactory() {
	if (FAILED(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(m_dwriteFactory),
		reinterpret_cast<IUnknown**>(m_dwriteFactory.ReleaseAndGetAddressOf())))) {
		return false;
	}
	return true;
}
bool D2D::CreateDxgiSurfaceRenderTarget(IDXGISwapChain* swapChain) {
	HRESULT hr;
	hr = swapChain->GetBuffer(0, __uuidof(IDXGISurface), &m_dxgiSurface);
	if (FAILED(hr)) return false;
	
	// D2D RenderTarget 속성 설정
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE)
	);

	// RenderTarget 생성
	hr = m_factory->CreateDxgiSurfaceRenderTarget(
		m_dxgiSurface.Get(),
		props,
		&m_renderTarget
	);
 

	if (FAILED(hr)) return false;
	return true;
}
bool D2D::Init(HWND hwnd, IDXGISwapChain* swapChain, ID3D11Device* m_d3dDevice) {

	m_hwnd = hwnd;


	if (!CreateD2DFactory()) return false;
	if (!CreateDWriteFactory()) return false;
	if (!CreateDxgiSurfaceRenderTarget(swapChain)) return false;
	

	{
		auto& m_d2dDevice = this->m_device;
		auto& m_d2dContext = this->m_deviceContext;
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;

		//hr = m_d3dDevice (&dxgiDevice);

		// (3) DXGI 디바이스로 D2D 디바이스 생성
		//hr = m_factory->CreateDevice(
		//	dxgiDevice.Get(), &m_d2dDevice);

		// (4) D2D 디바이스 컨텍스트 생성 (실제 그리기를 수행할 객체)
		//hr = m_d2dDevice->CreateDeviceContext(
		//	D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		//	&m_d2dContext
		//);
		//if (dxgiDevice) dxgiDevice->Release();
	}

	// Dwrite 팩토리를 생성함.
	


	RECT r;
	GetClientRect(hwnd, &r);

	/*hr = factory->CreateHwndRenderTarget(
		RenderTargetProperties(),
		HwndRenderTargetProperties(
			hwnd, SizeU(r.right - r.left, r.bottom - r.top)
		),
		&target);
	if (FAILED(hr)) return 0;

	target->CreateSolidColorBrush(D2D1::ColorF(0xffffff), &brush);
	*/
	m_solidBrush = CreateSolidBrush(D2D1::ColorF(0xffffff));
	if (!CreateFontDevice()) return false;

	if (!InitWICFactory()) return false;
	return true;
}

ID2D1Bitmap* D2D::LoadWICImageFromFile(const wchar_t* dir) {

	HRESULT hr = 0;
	IWICBitmapDecoder* pDecoder = nullptr;
	ID2D1Bitmap* pBitmap = nullptr;
	IWICBitmapFrameDecode* pFrame = nullptr;
	IWICFormatConverter* pConverter = nullptr;

	hr = m_wicFactory->CreateDecoderFromFilename(dir, nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnDemand, &pDecoder);

	// MainWindow 멤버 변수로 선언

	if (SUCCEEDED(hr))
		hr = pDecoder->GetFrame(0, &pFrame);

	if (SUCCEEDED(hr))
		hr = m_wicFactory->CreateFormatConverter(&pConverter);

	if (SUCCEEDED(hr))
		hr = pConverter->Initialize(pFrame,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.0f,
			WICBitmapPaletteTypeCustom
		);


	if (SUCCEEDED(hr))
		hr = m_deviceContext->CreateBitmapFromWicBitmap
		//hr = target->CreateBitmapFromWicBitmap
		(pConverter, nullptr, &pBitmap);

	return pBitmap;
}

