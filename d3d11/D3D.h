#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include "UserDef.h"
#define USEIMGUI

#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>


#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <tchar.h>


#ifdef USED2D
#include "D2D.hpp"
#else
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <SimpleMath.h>
#include <WICTextureLoader.h>

#endif
#ifdef USEIMGUI
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#endif
#include <iostream>
#include <vector>
#include <memory>

//d3d11, dxgi.lib
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

using Microsoft::WRL::ComPtr;

struct ConstantBuffer {
	XMMATRIX mWorld;      // 물체의 위치/회전
	XMMATRIX mView;       // 카메라의 위치
	XMMATRIX mProjection; // 원근감
};
class WinApp;
class MyWinApp;
class D3D {
	friend class WinApp;
	friend class MyWinApp;

protected:

	HWND m_hwnd;
	ComPtr<ID3D11Device>		m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	ComPtr<IDXGISwapChain>		m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_backBufferView;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	ComPtr<ID3D11RasterizerState> m_rasterState;

	ComPtr<ID3D11Buffer> m_constantBuffer;


	static D3D* s_instance;


	ComPtr<ID3D11VertexShader> m_VS = nullptr;
	ComPtr<ID3D11PixelShader> m_PS = nullptr;
	ComPtr<ID3D11InputLayout> m_InputLayout = nullptr;

public:
#ifdef USED2D
	ComPtr<IWICImagingFactory> m_wicFactory;
#else
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
#endif
public:

	bool Init(HWND hwnd, int screenWidth, int screenHeight);
	bool CreateZBuffer(int,int);
	bool CreateConstantBuffer();

	ID3D11Device* GetDevice() {
		return m_device.Get();
	}
	ID3D11DeviceContext* GetDeviceContext() {
		return m_deviceContext.Get();
	}
	IDXGISwapChain* GetSwapChain() {
		return m_swapChain.Get();
	}
	ID3D11RenderTargetView* GetBackBufferView() {
		return m_backBufferView.Get();
	}
	ComPtr<ID3D11DepthStencilView> GetDepthStencilView() 
		{ return m_depthStencilView; }

	ID3D11RasterizerState* GetRasterState() {
		return m_rasterState.Get();
	}

public:
	D3D() {
		m_hwnd = nullptr;
		m_device = nullptr;
	}
	~D3D() {

	}
	static void CreateInstance() {
		DeleteInstance();
		s_instance = new D3D();
	}
	static void DeleteInstance() {
		if (s_instance != nullptr) {
			delete s_instance;
			s_instance = nullptr;
		}
	}
	static D3D& GetInstance() {
		return *s_instance;
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

#ifdef USED2D/*
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
#undef AssertHR*/
#else
	bool InitSpriteFont();
	DirectX::SpriteBatch* GetSpriteBatch() {
		return m_spriteBatch.get();
	}
	DirectX::SpriteFont* GetSpriteFont() {
		return m_spriteFont.get();
	}
#endif

};

extern void OutputDebugStringFormat(const TCHAR* fstr, ...);