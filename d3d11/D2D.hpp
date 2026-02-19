#pragma once

#ifdef USED2D
#include <math.h>

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <Wincodec.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "windowscodecs")
using namespace D2D1;
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

typedef ID2D1SolidColorBrush* SolidBrush;
typedef D2D1_POINT_2F Point2;
//static void OutputDebugStringFormat(const TCHAR* fstr, ...);
class D2D {
private:
	ID2D1Device* m_device = nullptr;
	ID2D1DeviceContext* m_deviceContext = nullptr;
	ID2D1Factory1* factory;
	//ID2D1HwndRenderTarget* target;
	SolidBrush solidBrush;

	IDWriteFactory* write_factory;
	IDWriteTextFormat* text_format;
	IWICImagingFactory* wic_factory;


	HWND m_hwnd;
public:
	D2D() {
		factory = 0;
		//target = 0;
		solidBrush = 0;

		write_factory = 0;
		text_format = 0;
		wic_factory = 0;

		m_hwnd = 0;

	}
	~D2D() {

		SAFE_RELEASE(m_deviceContext);
		SAFE_RELEASE(m_device);

		SAFE_RELEASE(factory);
		//SAFE_RELEASE(target);
		SAFE_RELEASE(solidBrush);
		SAFE_RELEASE(text_format);
		SAFE_RELEASE(write_factory);
		SAFE_RELEASE(wic_factory);

		CoUninitialize();
	}



	bool Init(HWND hwnd, Microsoft::WRL::ComPtr<ID3D11Device>& m_d3dDevice) {
		m_hwnd = hwnd;
		HRESULT hr;
		D2D1_FACTORY_OPTIONS d2dOptions = {};

#ifdef _DEBUG
		d2dOptions.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
#endif

		hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
		if (FAILED(hr)) return false;

		hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory1),
			&d2dOptions,
			(void**)&factory);
		if (FAILED(hr)) return false;


		{
			auto& m_d2dDevice = this->m_device;
			auto& m_d2dContext = this->m_deviceContext;
			Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;

			hr = m_d3dDevice.As(&dxgiDevice);

			// (3) DXGI 디바이스로 D2D 디바이스 생성
			hr = factory->CreateDevice(
				dxgiDevice.Get(), &m_d2dDevice);

			// (4) D2D 디바이스 컨텍스트 생성 (실제 그리기를 수행할 객체)
			hr = m_d2dDevice->CreateDeviceContext(
				D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
				&m_d2dContext
			);
			//if (dxgiDevice) dxgiDevice->Release();
		}

		// Dwrite 팩토리를 생성함.
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(write_factory),
			reinterpret_cast<IUnknown**>(&write_factory)
		);
		if (FAILED(hr)) return 0;


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
		m_deviceContext->CreateSolidColorBrush(D2D1::ColorF(0xffffff), &solidBrush);

		hr = this->CreateFontDevice();
		if (FAILED(hr)) return 0;



		return true;
	}
	bool InitWICFactory() {
		return CoCreateInstance(CLSID_WICImagingFactory, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic_factory));
	}
	ID2D1Bitmap* LoadWICImageFromFile(const wchar_t* dir) {

		HRESULT hr = 0;
		IWICBitmapDecoder* pDecoder = nullptr;
		ID2D1Bitmap* pBitmap = nullptr;
		IWICBitmapFrameDecode* pFrame = nullptr;
		IWICFormatConverter* pConverter = nullptr;

		hr = wic_factory->CreateDecoderFromFilename(dir, nullptr, GENERIC_READ,
			WICDecodeMetadataCacheOnDemand, &pDecoder);

		// MainWindow 멤버 변수로 선언

		if (SUCCEEDED(hr))
			hr = pDecoder->GetFrame(0, &pFrame);

		if (SUCCEEDED(hr))
			hr = wic_factory->CreateFormatConverter(&pConverter);

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

	HRESULT CreateFontDevice() {

		static const WCHAR msc_fontName[] = L"Consolas";
		static const FLOAT msc_fontSize = 50;
		HRESULT hr;


		//DirectWrite 텍스트 포맷 객체를 생성함.
		hr = write_factory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			&text_format
		);

		if (SUCCEEDED(hr))
		{
			// 텍스트를 수평으로 중앙 정렬하고 수직으로도 중앙 정렬함.
			text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}

		return hr;
	}


	/*void BeginDraw() {
		target->BeginDraw();
	}
	HRESULT EndDraw() {
		return target->EndDraw();
	}
	void Clear(ColorF color) {
		target->Clear(color);
	}
	void DrawSquare(D2D1_RECT_F rect, ColorF color, bool fill_flag = false) {
		brush->SetColor(color);

		if (fill_flag)target->FillRectangle(rect, brush);
		target->DrawRectangle(rect, brush);
		//brush->Release();
	}

	void FillSquare(D2D1_RECT_F rect, ColorF color) {
		brush->SetColor(color);
		target->FillRectangle(rect, brush);
		//brush->Release();
	}


	*/

	/*HRESULT CreateDeviceResources()
	{
		HRESULT hr = S_OK;

		if (!m_deviceContext) {


			RECT rc;
			GetClientRect(m_hwnd, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top
			);

			// Create D2D RenderTarget
			hr = factory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(m_hwnd, size),
				&target
			);
		}
		return hr;
	}*/
	/*void DiscardDeviceResources()
	{
		SAFE_RELEASE(target);
	}*/

	//ID2D1HwndRenderTarget* GetTarget() { return target; }
	ID2D1Factory* GetFactory() { return factory; }
	ID2D1Device* GetDevice() { return m_device; }
	ID2D1DeviceContext* GetDeviceContext() { return m_deviceContext; }
	SolidBrush GetSolidBrush() { return solidBrush; }

};
#endif