
#ifndef __D3D_H__
#define __D3D_H__
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

//#include "UserDef.h"
#define USEIMGUI

#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>


#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <tchar.h>

//#include "D2D.hpp"

using namespace DirectX;

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <iostream>
#include <vector>
#include <memory>

//d3d11, dxgi.lib
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib, "d3dcompiler.lib")


using Microsoft::WRL::ComPtr;

class D3D {
public:
	static D3D* s_instance;

	HWND m_hwnd;
	ComPtr<ID3D11Device>		m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	ComPtr<IDXGISwapChain>		m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_rtview;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11RasterizerState> m_rasterState;

	ComPtr<ID3D11VertexShader> m_VS = nullptr;
	ComPtr<ID3D11PixelShader> m_PS = nullptr;
	ComPtr<ID3D11InputLayout> m_InputLayout = nullptr;


	ComPtr<IWICImagingFactory> m_wicFactory;

	bool Init(HWND hwnd, int screenWidth, int screenHeight);
	bool CreateZBuffer(int, int);
public:
	D3D() {
		m_hwnd = nullptr;
		m_device = nullptr;
	}
	~D3D() {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
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

};

extern void OutputDebugStringFormat(const TCHAR* fstr, ...);

#endif