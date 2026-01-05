#include "d2d.hpp"

D2D::~D2D() {
    this->device.Reset();
    this->deviceContext.Reset();
    this->targetBitmap.Reset();
    this->factory.Reset();
    
    SAFE_RELEASE(solidBrush);
    SAFE_RELEASE(textFormat);
    SAFE_RELEASE(writeFactory);
    SAFE_RELEASE(wicFactory);
    
    CoUninitialize();
}

bool D2D::Init(HWND hwnd, ComPtr<ID3D11Device>& d3dDevice, ComPtr<IDXGISwapChain>& swapChain) {
    if (!hwnd || !d3dDevice) return false;

    this->hwnd = hwnd;
    HRESULT hr;

    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        OutputDebugStringFormat(_T("Already coInitialized"));
    }

    D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    // D2D Factory 생성
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&factory);
    if (FAILED(hr)) return false;


    // D2D Device 생성
    ComPtr<IDXGIDevice> dxgiDevice;
    hr = d3dDevice.As(&dxgiDevice);
    if (FAILED(hr)) return false;

    hr = factory->CreateDevice(dxgiDevice.Get(), &this->device);
    if (FAILED(hr)) return false;


    hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &deviceContext);
    if (FAILED(hr)) return false;

    // DirectWrite Factory 생성
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFactory);
    if (FAILED(hr)) return false;


    // 7. SwapChain의 백버퍼를 D2D의 비트맵 타겟으로 설정 ★★★
    ComPtr<IDXGISurface> dxgiSurface;
    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface)))) // 백버퍼를 Surface로 가져옴
        return false;

    // Surface 설정 (DPI 등)
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
    
    // DXGI Surface -> D2D Bitmap 변환
    if (FAILED(deviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &targetBitmap)))
        return false;

    // D2D Context에 타겟 지정 "이제부터 여기에 그린다"
    deviceContext->SetTarget(targetBitmap.Get());

    // Solid Brush 생성
    hr = deviceContext->CreateSolidColorBrush(D2D1::ColorF(0xffffff), &solidBrush);
    if (FAILED(hr)) return false;

    hr = CreateFontDevice();
    if (FAILED(hr)) return false;
    return true;
}

bool D2D::InitWICFactory() {
    return SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory)));
}

ID2D1Bitmap* D2D::LoadWICImageFromFile(const wchar_t* filePath) {
    if (!filePath || !wicFactory || !deviceContext) return nullptr;

    HRESULT hr = S_OK;
    ID2D1Bitmap* bitmap = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;

    hr = wicFactory->CreateDecoderFromFilename(filePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return nullptr;

    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return nullptr;

    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return nullptr;

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return nullptr;

    hr = deviceContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap);
    if (FAILED(hr)) return nullptr;

    return bitmap;
}

HRESULT D2D::CreateFontDevice() {
    static const WCHAR fontName[] = L"Consolas";
    static const FLOAT fontSize = 50;

    HRESULT hr = writeFactory->CreateTextFormat(
        fontName, nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"", &textFormat
    );

    if (SUCCEEDED(hr)) {
        textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    return hr;
}