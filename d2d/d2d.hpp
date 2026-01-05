
#ifndef __D2D_H__
#define __D2D_H__
#define USED2D

#include "d3d.hpp"
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

#define SAFE_RELEASE(p) { if((p)) { (p)->Release(); (p) = nullptr; } }

typedef ID2D1SolidColorBrush* SolidBrush;
typedef D2D1_POINT_2F Point2;

class D2D {
public:
    ComPtr<ID2D1Factory1> factory;
    ComPtr < ID2D1Device> device;
    ComPtr<ID2D1DeviceContext> deviceContext;
    ComPtr < ID2D1Bitmap1> targetBitmap;

    SolidBrush solidBrush = nullptr;
    IDWriteFactory* writeFactory = nullptr;
    IDWriteTextFormat* textFormat = nullptr;

    IWICImagingFactory* wicFactory = nullptr;
    HWND hwnd = nullptr;

    HRESULT CreateFontDevice();

public:
    D2D() = default;
    ~D2D();

    bool Init(HWND hwnd, Microsoft::WRL::ComPtr<ID3D11Device>& d3dDevice, ComPtr<IDXGISwapChain>& swapChain);
    bool InitWICFactory();
    ID2D1Bitmap* LoadWICImageFromFile(const wchar_t* filePath);

    // Drawing API
    void BeginDraw() { if (deviceContext) deviceContext->BeginDraw(); }
    HRESULT EndDraw() { return deviceContext ? deviceContext->EndDraw() : E_FAIL; }
    void Clear(D2D1::ColorF color) { if (deviceContext) deviceContext->Clear(color); }

};

#endif