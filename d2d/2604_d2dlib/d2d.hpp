#pragma once
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <wincodec.h>
#include <d3d11.h>
#include <d2d1.h>

#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl.h>
#include <TCHAR.h>


#ifdef __cplusplus
#define s_cast(T, V) static_cast<T>(V)
#define c_cast(T, V) const_cast<T>(V)
#define r_cast(T, V) reinterpret_cast<T>(V)
#define d_cast(T, V) dynamic_cast<T>(V)
#else
#define s_cast(T, V) ((T)(V))
#define c_cast(T, V) ((T)(V))
#define r_cast(T, V) ((T)(V))
#define d_cast(T, V) ((T)(V))
#endif

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

#define SAFE_RELEASE(p) { if((p!=nullptr)) { (p)->Release(); (p) = nullptr; } }

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib, "d3dcompiler.lib")


using Microsoft::WRL::ComPtr;


static void OutputDebugStringFormat(const TCHAR* fstr, ...) {
    TCHAR buffer[256];
    va_list li;
    va_start(li, fstr);
    _vstprintf(buffer, fstr, li);
    va_end(li);
    OutputDebugString(buffer);
    OutputDebugString(_T("\n"));
}


class D3D {
public:


    D3D() = default;
    ~D3D();


    template <typename T1, typename T>
    static HRESULT QueryInterface(T1 LHS, T* RHS) {
        return LHS->QueryInterface(__uuidof(T), (void**)(RHS));
    }

    bool Init(HWND hwnd, int screenWidth, int screenHeight);
    bool CreateZBuffer(int, int);


    //bool InitImGui();
public:

    HWND mHwnd;
    ID3D11Device* mDevice = nullptr;
    ID3D11DeviceContext* mDeviceContext = nullptr;
    IDXGISwapChain* mSwapChain = nullptr;
    ID3D11RenderTargetView* mRtView = nullptr;
    ID3D11DepthStencilView* mDepthStencilView = nullptr;
    ID3D11RasterizerState* mRasterState = nullptr;
    ID3D11VertexShader* mVS = nullptr;
    ID3D11PixelShader* mPS = nullptr;
    ID3D11InputLayout* mInputLayout = nullptr;
    IWICImagingFactory* mWicFactory = nullptr;
};
class D2D {

public:
    HRESULT CreateFontDevice();

    D2D() = default;
    ~D2D();

    bool Init(D3D* d3d, HWND hwnd, ID3D11Device* d3dDevice, IDXGISwapChain* swapChain);
    bool InitWICFactory();
    bool LoadWICImageFromFile(const wchar_t* filePath, ID2D1Bitmap** _bitmap);
    bool LoadWICImageFromMemory(void* pData, DWORD dataSize, ID2D1Bitmap** _bitmap);
    // Drawing API
    void BeginDraw() { if (mDeviceContext) mDeviceContext->BeginDraw(); }
    HRESULT EndDraw() { return mDeviceContext ? mDeviceContext->EndDraw() : E_FAIL; }
    void Clear(D2D1::ColorF color) { if (mDeviceContext) mDeviceContext->Clear(color); }

    
        void ResizeApp(HWND hwnd, int width, int height) {
            D3D* d3d = this->mD3D;

            // 1. 꼬리 자르기: 백버퍼를 참조하는 모든 객체 해제
            // (이거 안 하고 ResizeBuffers 하면 에러 뜹니다)
            mDeviceContext->SetTarget(nullptr); // D2D 문맥에서 타겟 떼기
            SAFE_RELEASE(mTargetBitmap);
            SAFE_RELEASE(d3d->mRtView);
            // 2. 스왑체인 크기 변경
            d3d->mSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

            // 3. 다시 연결 (Init 코드와 비슷)
            // 3-1. D3D RTV 재생성
            ComPtr<ID3D11Texture2D> backBuffer;
            d3d->mSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
            d3d->mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &d3d->mRtView);

            // 3-2. D2D 비트맵 재생성
            ComPtr<IDXGISurface> dxgiSurface;
            d3d->mSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));

            float dpiX, dpiY;
            UINT dpi = GetDpiForWindow(hwnd);
            dpiX = (float)(dpi);
            dpiY = (float)(dpi);

            /*Windows7~
            HDC hdc = GetDC(m_hwnd);
            dpiX = static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSX));
            dpiY = static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSY));
            ReleaseDC(m_hwnd, hdc);

            */

            //g_d2d->factory->GetDesktopDpi(&dpiX, &dpiY);
            D2D1_BITMAP_PROPERTIES1 bp = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                dpiX, dpiY
            );

            mDeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bp, &mTargetBitmap);
            mDeviceContext->SetTarget(mTargetBitmap);

            //화면 크기 맞춤
            D3D11_VIEWPORT vp;
            vp.Width = (float)width;
            vp.Height = (float)height;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            d3d->mDeviceContext->RSSetViewports(1, &vp);
        }
    
public:
    D3D* mD3D;

    ID2D1Factory1* mFactory = nullptr;
    ID2D1Device* mDevice = nullptr;
    ID2D1DeviceContext* mDeviceContext = nullptr;
    ID2D1DeviceContext3* mDC3 = nullptr;
    ID2D1Bitmap1* mTargetBitmap = nullptr;
    ID2D1SpriteBatch* mSpriteBatch = nullptr;
    ID2D1SpriteBatch* mSpriteBatchText = nullptr;
    ID2D1Bitmap* mBakedBitmap = nullptr;
    ID2D1Bitmap* mBakedBitmapText = nullptr;
    ID2D1DCRenderTarget* mDCRT = nullptr;
    //ID2D1SolidColorBrush* mSolidBrush = nullptr;

    IDWriteFactory* mDWFactory = nullptr;
    IDWriteTextFormat* mDWTextFormat = nullptr;
    IWICImagingFactory* mWICFactory = nullptr;
};

