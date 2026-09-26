#pragma once
#include "DirectxHead.hpp"
//typedef ID2D1SolidColorBrush* D2DSolidBrush;
//static void OutputDebugStringFormat(const TCHAR* fstr, ...);
class D2D {

public:
    D2D() {
        m_hwnd = 0;
    }
    ~D2D() {
        
        //m_renderTarget.Reset();
        //m_dxgiSurface.Reset();

        CoUninitialize();
    }

    bool Init(HWND hwnd, IDXGISwapChain* swapChain, ID3D11Device* m_d3dDevice);
    ID2D1Bitmap* LoadWICImageFromFile(const wchar_t* dir);
    ComPtr<ID2D1Factory>   GetFactory() const { return m_factory; }
    ComPtr<ID2D1Device>	   GetDevice() const { return m_device; }
    ComPtr<ID2D1DeviceContext>	 GetDeviceContext() const { return m_deviceContext; }
    ComPtr<ID2D1SolidColorBrush> GetSolidBrush() const { return m_solidBrush; }
    ComPtr<ID2D1RenderTarget>	 GetRenderTarget() const { return m_renderTarget; }
    ID2D1SolidColorBrush* CreateSolidBrush(const D2D1_COLOR_F& col) {
        ID2D1SolidColorBrush* brush = nullptr;
        if (m_renderTarget) {
            m_renderTarget->CreateSolidColorBrush(col, &brush);
        }
        return brush;
    }

    
    
    IDWriteTextFormat* CreateTextFormat(const wchar_t* fontName, float fontSize, bool bCenter=true) {
        IDWriteTextFormat* txtform;
        if (m_dwriteFactory == nullptr) return nullptr;

        m_dwriteFactory->CreateTextFormat(
            fontName,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"en-us", //locale
            &txtform
        );

        if (bCenter)
        {
            // 텍스트를 수평으로 중앙 정렬하고 수직으로도 중앙 정렬함.
            txtform->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            txtform->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
        return txtform;

    }

protected:
    bool CreateD2DFactory();
    bool CreateDWriteFactory();
    bool CreateDxgiSurfaceRenderTarget(IDXGISwapChain* swapChain);
    bool CreateFontDevice() {
        static const WCHAR msc_fontName[] = L"Consolas";
        static const FLOAT msc_fontSize = 50;

        m_textFormat = CreateTextFormat(msc_fontName, msc_fontSize);
        if (m_textFormat == nullptr) return false;
        return true;
    }
    bool InitWICFactory() {
        return SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory)));
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

private:
    ComPtr<ID2D1Factory1> m_factory = nullptr;
    ComPtr<ID2D1Device> m_device = nullptr;
    ComPtr<ID2D1DeviceContext> m_deviceContext = nullptr;
    //ID2D1HwndRenderTarget* target;
    ComPtr<ID2D1SolidColorBrush> m_solidBrush = nullptr;

    ComPtr<IDWriteFactory>	    m_dwriteFactory = nullptr;
    ComPtr<IDWriteTextFormat> m_textFormat = nullptr;
    ComPtr<IWICImagingFactory> m_wicFactory = nullptr;


    ComPtr<ID2D1RenderTarget> m_renderTarget = nullptr;
    ComPtr<IDXGISurface> m_dxgiSurface = nullptr;


    HWND m_hwnd;
};