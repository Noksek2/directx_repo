#include "D3D.h"
//https://gamesgard.com/directx11_lesson02/
D3D* D3D::s_instance;
bool D3D::Init(HWND hwnd, int screenWidth, int screenHeight) {
	m_hwnd = hwnd;

	/* Initialize D3D
	*/

	ComPtr<IDXGIFactory> factory;
	//Create Factory
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
		return false;
	}
	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
#ifdef USED2D
	createFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif
	//featureLevel
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,	// Direct3D 11.1  ShaderModel 5
		D3D_FEATURE_LEVEL_11_0,	// Direct3D 11    ShaderModel 5
		D3D_FEATURE_LEVEL_10_1,	// Direct3D 10.1  ShaderModel 4
		D3D_FEATURE_LEVEL_10_0,	// Direct3D 10.0  ShaderModel 4
		D3D_FEATURE_LEVEL_9_3,	// Direct3D 9.3   ShaderModel 3
		D3D_FEATURE_LEVEL_9_2,	// Direct3D 9.2   ShaderModel 3
		D3D_FEATURE_LEVEL_9_1,	// Direct3D 9.1   ShaderModel 3
	};
	D3D_FEATURE_LEVEL featureLevel;
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		&m_device,
		&featureLevel,
		&m_deviceContext))) {
		return false;
	}

	//swap chain desc
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	//SET SWAPCHAINDESC
	{
		auto& s = scDesc;
		{
			auto& buf = s.BufferDesc;
			buf.Width = screenWidth;
			buf.Height = screenHeight;
			buf.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			buf.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			buf.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		}
		s.SampleDesc.Count = 1;
		s.SampleDesc.Quality = 0;

		s.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		s.BufferCount = 2;

		s.OutputWindow = hwnd;
		s.Windowed = true;

		s.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		s.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	}
	if (FAILED(factory->CreateSwapChain(
		m_device.Get(), &scDesc, &m_swapChain))) {
		return false;
	}

	//BACK BUFFER
	ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
		return false;
	}


	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = scDesc.BufferDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), &rtvDesc, &m_backBufferView)))
	{
		return false;
	}


	//Create ZBuffer
	if (!this->CreateZBuffer(screenWidth, screenHeight)) {
		return false;
	}


	m_deviceContext->OMSetRenderTargets(1,
		m_backBufferView.GetAddressOf(), m_depthStencilView.Get());
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	//viewport
	D3D11_VIEWPORT vp = { 0.f, 0.f,
		(float)screenWidth,(float)screenHeight,
		0.f,1.f };
	m_deviceContext->RSSetViewports(1, &vp);


	ComPtr<ID3DBlob> errorBlob = nullptr; // 에러 메시지 받을 그릇
	//Shaer InputLayout Setting
	ComPtr<ID3DBlob> compiledVS;
	if (FAILED(D3DCompileFromFile(L"Shader/SpriteShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &compiledVS, &errorBlob)))
	{

		if (errorBlob) {
			MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Shader Compile Error", MB_OK);
		}
		else {
			// 에러 메시지도 없이 실패했다면 -> 십중팔구 "파일 못 찾음"
			MessageBox(nullptr, L"셰이더 파일을 찾을 수 없습니다.\n경로를 확인하세요.", L"File Not Found", MB_OK);
		}
		return false;
	}
	// ピクセルシェーダーを読み込み＆コンパイル
	

	ComPtr<ID3DBlob> compiledPS;
	if (FAILED(D3DCompileFromFile(L"Shader/SpriteShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &compiledPS, &errorBlob)))
	{
		return false;
	}
	// ピクセルシェーダーを読み込み＆コンパイル



	// 頂点シェーダー作成
	if (FAILED(m_device->CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &m_VS)))
	{
		return false;
	}
	
	// ピクセルシェーダー作成
	if (FAILED(m_device->CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &m_PS)))
	{
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	constexpr int layout_len = 2;

	// 頂点インプットレイアウト作成
	if (FAILED(m_device->CreateInputLayout(layout, layout_len, compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &m_InputLayout)))
		return false;

	//시저링 방지 -> 화면 밖도 출력 가능
	D3D11_RASTERIZER_DESC desc = {};
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE;
	desc.ScissorEnable = FALSE; // ⬅️⬅️⬅️ 이게 핵심입니다! (시저링 비활성화)
	desc.DepthClipEnable = TRUE; // Z클리핑은 켜도 2D와 무관합니다.

	// "시저링 끈" 래스터라이저 상태 객체 생성
	if (FAILED(m_device->CreateRasterizerState(&desc, m_rasterState.GetAddressOf()))) {
		MessageBox(0, L"Raster", L"", MB_ICONERROR);
		return false;
	}

	if (!CreateConstantBuffer()) return false;

	return true;
}

bool D3D::CreateZBuffer(int screenWidth, int screenHeight) {
	ComPtr<ID3D11Texture2D> depthBufferTexture;
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = screenWidth;  // 백 버퍼와 동일한 크기
	depthDesc.Height = screenHeight; // 백 버퍼와 동일한 크기
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 깊이(24bit) + 스텐실(8bit) 포맷
	depthDesc.SampleDesc.Count = 1; // SampleDesc는 스왑체인과 일치해야 함
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // "이 텍스처는 깊이 버퍼로 쓰겠다"
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	if (FAILED(m_device->CreateTexture2D(&depthDesc, nullptr, &depthBufferTexture)))
	{
		return false;
	}

	// (2) 텍스처를 기반으로 깊이 스텐실 "뷰(View)" 생성
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(m_device->CreateDepthStencilView(depthBufferTexture.Get(), &dsvDesc, &m_depthStencilView)))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC dssDesc = {};
	dssDesc.DepthEnable = TRUE;                      // 깊이 테스트 사용
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 값 쓰기 허용
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;       // 현재 값보다 작으면(가까우면) 통과

	if (FAILED(m_device->CreateDepthStencilState(&dssDesc, &m_depthStencilState))) {
		return false;
	}
	
	return true;
}
bool D3D::CreateConstantBuffer() {
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 중요!
	cbDesc.CPUAccessFlags = 0;

	if (FAILED(m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer))) {
		return false;
	}
	return true;
}

#ifdef USED2D
#else
bool D3D::InitSpriteFont() {
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_deviceContext.Get());
	try {
		m_spriteFont = std::make_unique<DirectX::SpriteFont>(m_device.Get(), L"ComicSans.spriteFont");
	}
	catch (std::exception& e) {
		// 폰트 로드 실패!
		OutputDebugStringA(e.what());
		MessageBoxA(nullptr, e.what(), "Font Load Error", MB_OK);
		// ... 실패 처리 ...
		return false;
	}

	return true;
}
#endif
void OutputDebugStringFormat(const TCHAR* fstr, ...) {
	TCHAR buffer[0x160];
	va_list li;
	va_start(li, fstr);
	_vstprintf(buffer, fstr, li);
	va_end(li);
	OutputDebugString(buffer);
	OutputDebugString(_T("\n"));
}

/*


*/