#include "d2d.hpp"
/* D3D */
bool D3D::Init(HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT hr = TRUE;
    mHwnd = hwnd;

    UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 2;
    {
        auto& buf = scDesc.BufferDesc;
        buf.Width = screenWidth;
        buf.Height = screenHeight;
        buf.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        //buf.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        //buf.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    }
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;

    scDesc.OutputWindow = hwnd;
    scDesc.Windowed = true;

    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    //		s.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags,
        nullptr, 0, D3D11_SDK_VERSION, &scDesc,
        &mSwapChain, &mDevice, nullptr, &mDeviceContext)))
        return false;



    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
        goto lRelease;

    if (FAILED(mDevice->CreateRenderTargetView(backBuffer, nullptr, &mRtView)))
        goto lRelease;

lRelease:
    SAFE_RELEASE(backBuffer);
    return hr;
}
D3D::~D3D() {

    SAFE_RELEASE(mInputLayout);
    SAFE_RELEASE(mVS);
    SAFE_RELEASE(mPS);
    SAFE_RELEASE(mRasterState);
    
    SAFE_RELEASE(mRtView);//이 새끼
    SAFE_RELEASE(mDepthStencilView);


    if (mDeviceContext) {
        mDeviceContext->ClearState();
        mDeviceContext->Flush();
        mDeviceContext->Release();
        mDeviceContext = nullptr;

    }
    SAFE_RELEASE(mDeviceContext);
    SAFE_RELEASE(mSwapChain);
    
    SAFE_RELEASE(mDevice);
    SAFE_RELEASE(mWicFactory);


}

/* D2D */


//#define CHECK_FAILED(hr)
bool D2D::Init(D3D* d3d, HWND hwnd, ID3D11Device* d3dDevice, IDXGISwapChain* swapChain) {

    if (!hwnd || !d3dDevice) return false;

    HRESULT hr;
    IDXGIDevice* dxgiDevice = nullptr;
    IDXGISurface* dxgiSurface = nullptr;


    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
    D2D1_PIXEL_FORMAT fmt = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
    unsigned char data[4] = { 255, 255, 255, 255 };

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        0, 0,
        D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE, // GDI 호환성 필수
        D2D1_FEATURE_LEVEL_DEFAULT
    );


    mD3D = d3d;


    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        OutputDebugStringFormat(_T("Already coInitialized"));
        return false;
    }

    D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    // D2D Factory 생성
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&mFactory);
    if (FAILED(hr)) goto lRelease;

    // D2D Device 생성
    hr = D3D::QueryInterface(d3dDevice, &dxgiDevice);
    //hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    //hr = d3dDevice.As(&dxgiDevice);
    if (FAILED(hr)) goto lRelease;

    hr = mFactory->CreateDevice(dxgiDevice, &mDevice);
    if (FAILED(hr)) goto lRelease;


    hr = mDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mDeviceContext);
    if (FAILED(hr)) goto lRelease;

    // DirectWrite Factory 생성
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&mDWFactory);
    if (FAILED(hr)) goto lRelease;


    // 7. SwapChain의 백버퍼를 D2D의 비트맵 타겟으로 설정

    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface)))) // 백버퍼를 Surface로 가져옴
        goto lRelease;

    // Surface 설정 (DPI 등)


    // DXGI Surface -> D2D Bitmap 변환
    if (FAILED(mDeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bitmapProperties, &mTargetBitmap)))
        goto lRelease;

    // D2D Context에 타겟 지정 "이제부터 여기에 그린다"
    mDeviceContext->SetTarget(mTargetBitmap);

    
    hr = CreateFontDevice();
    if (FAILED(hr)) goto lRelease;

    hr = D3D::QueryInterface(mDeviceContext, &mDC3);
    //hr = deviceContext->QueryInterface(__uuidof(ID2D1DeviceContext3), (void**)&this->dc3);
    if (FAILED(hr)) goto lRelease;

    hr = mDC3->CreateSpriteBatch(&mSpriteBatch);
    if (FAILED(hr)) goto lRelease;

    hr = mDeviceContext->CreateBitmap(D2D1::SizeU(1, 1), data, 4, D2D1::BitmapProperties(fmt), &mBakedBitmap);

    
    mFactory->CreateDCRenderTarget(&props, &mDCRT);
lRelease:
    SAFE_RELEASE(dxgiDevice);
    SAFE_RELEASE(dxgiSurface);

    return (SUCCEEDED(hr));
}

bool D2D::InitWICFactory() {
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, 
        nullptr,
        CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&mWICFactory));
    return SUCCEEDED(hr);
}

bool D2D::LoadWICImageFromFile(const wchar_t* filePath, ID2D1Bitmap** _bitmap) {
    if (!filePath || !mWICFactory || !mDeviceContext) return false;

    HRESULT hr = S_OK;
    ID2D1Bitmap* bitmap = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;

    hr = mWICFactory->CreateDecoderFromFilename(filePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return false;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return false;

    hr = mWICFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return false;

    hr = converter->Initialize(frame.Get(), 
           GUID_WICPixelFormat32bppPBGRA, //Compatible with D2D 32 BGRA Format
           WICBitmapDitherTypeNone, 
           nullptr,
           0.0f,
           WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return false;

    hr = mDeviceContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap);
    if (FAILED(hr))  return false;

    *_bitmap = bitmap;
    return true;
}
bool D2D::LoadWICImageFromMemory(void* pData, DWORD dataSize, ID2D1Bitmap** _bitmap)
{
    if (!mWICFactory || !mDeviceContext) return false;

    IWICStream* pStream = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pSource = nullptr;
    IWICFormatConverter* pConverter = nullptr;
    ID2D1Bitmap* pBitmap = nullptr;

    // 1. WIC 스트림 생성
    HRESULT hr = mWICFactory->CreateStream(&pStream);

    // 2. 메모리 버퍼를 스트림에 연결
    if (SUCCEEDED(hr))
    {
        hr = pStream->InitializeFromMemory(
            r_cast(BYTE*, pData),
            dataSize
        );
    }

    // 3. 스트림으로부터 디코더 생성
    if (SUCCEEDED(hr))
    {
        hr = mWICFactory->CreateDecoderFromStream(
            pStream,
            nullptr,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
        );
    }

    // 4. 프레임 추출 및 포맷 변환 (파일 로드 로직과 동일)
    if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, &pSource);
    if (SUCCEEDED(hr)) hr = mWICFactory->CreateFormatConverter(&pConverter);
    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA,
                                    WICBitmapDitherTypeNone, nullptr, 0.0f,
                                    WICBitmapPaletteTypeMedianCut);
    }

    // 5. D2D 비트맵 생성
    if (SUCCEEDED(hr))
    {
        hr = mDeviceContext->CreateBitmapFromWicBitmap(pConverter, nullptr, &pBitmap);
    }

    *_bitmap = pBitmap;
    // 자원 해제
    if (pStream) pStream->Release();
    if (pDecoder) pDecoder->Release();
    if (pSource) pSource->Release();
    if (pConverter) pConverter->Release();

    return (pBitmap != nullptr);
}
HRESULT D2D::CreateFontDevice() {
    static const WCHAR fontName[] = L"Consolas";
    static const FLOAT fontSize = 50;

    HRESULT hr = mDWFactory->CreateTextFormat(
        fontName, nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"", &mDWTextFormat
    );

    if (SUCCEEDED(hr)) {
        mDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        mDWTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    return hr;
}

D2D::~D2D() {
    
    
    SAFE_RELEASE(mBakedBitmap);
    SAFE_RELEASE(mBakedBitmapText);
    //SAFE_RELEASE(mSolidBrush);
    SAFE_RELEASE(mTargetBitmap);


    SAFE_RELEASE(mSpriteBatch);
    SAFE_RELEASE(mSpriteBatchText);


    SAFE_RELEASE(mDWTextFormat);

    SAFE_RELEASE(mDCRT);
    SAFE_RELEASE(mDC3); //이걸 모름
    SAFE_RELEASE(mDeviceContext);
    
    SAFE_RELEASE(mDevice);
    

    SAFE_RELEASE(mWICFactory);
    SAFE_RELEASE(mDWFactory);
    SAFE_RELEASE(mFactory);

    
    if (mD3D) { delete mD3D; mD3D = nullptr; }
    CoUninitialize();
}

