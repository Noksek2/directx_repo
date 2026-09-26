#pragma once


#include "DirectxHead.hpp"

struct ConstantBuffer {
	XMMATRIX mWorld;      // 물체의 위치/회전
	XMMATRIX mView;       // 카메라의 위치
	XMMATRIX mProjection; // 원근감
};
class WinApp;
class D3D {
public:
	D3D() {
		m_hwnd = nullptr;
		m_device = nullptr;

		m_width = 800;
		m_height = 600;
		m_vsync = true;
	}
	~D3D() {

	}
	bool Init(HWND hwnd, int screenWidth, int screenHeight, bool vsync = true);
	ComPtr<ID3D11Device> GetDevice() const {
		return m_device;
	}
	ComPtr<ID3D11DeviceContext> GetDeviceContext() const {
		return m_deviceContext;
	}
	ComPtr <IDXGISwapChain> GetSwapChain() const {
		return m_swapChain;
	}
	ComPtr <ID3D11RenderTargetView> GetBackBufferView() const {
		return m_backBufferView;
	}
	ComPtr <ID3D11RenderTargetView> GetRenderTargetView() const {
		return m_backBufferView;
	}
	ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const {
		return m_depthStencilView;
	}

	ID3D11RasterizerState* GetRasterState() const {
		return m_rasterState.Get();
	}

#ifdef USEIMGUI

	bool InitImGui() {
		if (m_device.Get() == nullptr) {
			MessageBox(m_hwnd, L"Failed to D3D Initialize, or do not initialize yet", L"", MB_OK);
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
		ImGui_ImplWin32_Init(m_hwnd);
		ImGui_ImplDX11_Init(m_device.Get(), m_deviceContext.Get());

		// 4. (선택) 폰트 로드
		// io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Malgun.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesKorean());
		return true;
	}
#endif

private:
	bool CreateDeviceContext();
	bool CreateSwapChain();
	bool CreateRenderTarget();
	bool CreateDepthStencil();
	bool CreateZBuffer(int,int);
	bool CreateConstantBuffer();

	
	

#define AssertHR(RES) if(FAILED(RES)){return src;}
	static ComPtr<IWICBitmapSource> WICBitmapFromFile(wchar_t* path)
	{

		ComPtr<IWICBitmapSource> src = nullptr;
		ComPtr<IWICImagingFactory> factory;
		AssertHR(CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&factory)
		));

		ComPtr<IWICBitmapDecoder> decoder;
		AssertHR(factory->CreateDecoderFromFilename(
			path,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&decoder
		));

		ComPtr<IWICBitmapFrameDecode> frame;
		AssertHR(decoder->GetFrame(0, &frame));

		// 32bit RGBAに変換
		ComPtr<IWICFormatConverter> converter;
		AssertHR(factory->CreateFormatConverter(&converter));
		AssertHR(converter->Initialize(
			frame.Get(),
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.0f,
			WICBitmapPaletteTypeCustom
		));

		return converter;

	}
#undef AssertHR

//	bool InitSpriteFont();
//	DirectX::SpriteBatch* GetSpriteBatch() {
//		return m_spriteBatch.get();
//	}
//	DirectX::SpriteFont* GetSpriteFont() {
//		return m_spriteFont.get();
//	}


protected:
	HWND m_hwnd;
	ComPtr<ID3D11Device>		    m_device = nullptr;
	ComPtr<ID3D11DeviceContext>     m_deviceContext = nullptr;
	ComPtr<IDXGISwapChain>		    m_swapChain = nullptr;
	ComPtr<ID3D11RenderTargetView>  m_backBufferView = nullptr;
	ComPtr<ID3D11DepthStencilView>  m_depthStencilView = nullptr;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState = nullptr;
	ComPtr<ID3D11RasterizerState>   m_rasterState = nullptr;
	ComPtr<ID3D11Texture2D>         m_depthStencilBuffer = nullptr;

	ComPtr<ID3D11Buffer> m_constantBuffer = nullptr;



	ComPtr<ID3D11VertexShader> m_VS = nullptr;
	ComPtr<ID3D11PixelShader> m_PS = nullptr;
	ComPtr<ID3D11InputLayout> m_InputLayout = nullptr;

	int m_width;
	int m_height;
	bool m_vsync;
public:
#ifdef USED2D
	ComPtr<IWICImagingFactory> m_wicFactory = nullptr;
#else
//	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
//	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
#endif

};

extern void OutputDebugStringFormat(const TCHAR* fstr, ...);