#include "d3d.hpp"
//https://gamesgard.com/directx11_lesson02/
D3D* D3D::s_instance;
bool D3D::Init(HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT hr=true;
	m_hwnd = hwnd;

	UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


	DXGI_SWAP_CHAIN_DESC scDesc = {};
	//SET SWAPCHAINDESC
	{
		auto& s = scDesc;
		s.BufferCount = 2;
		{
			auto& buf = s.BufferDesc;
			buf.Width = screenWidth;
			buf.Height = screenHeight;
			buf.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			//buf.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			//buf.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		}
		s.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		s.SampleDesc.Count = 1;
		s.SampleDesc.Quality = 0;

		s.OutputWindow = hwnd;
		s.Windowed = true;

		s.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		//		s.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	}
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags,
		nullptr, 0, D3D11_SDK_VERSION, &scDesc,
		&m_swapChain, &m_device, nullptr, &m_deviceContext)))
		return false;



	ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
		return false;

	if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtview)))
		return false;
	return hr;

}

void OutputDebugStringFormat(const TCHAR* fstr, ...) {
	TCHAR buffer[0x160];
	va_list li;
	va_start(li, fstr);
	_vstprintf(buffer, fstr, li);
	va_end(li);
	OutputDebugString(buffer);
	OutputDebugString(_T("\n"));
}