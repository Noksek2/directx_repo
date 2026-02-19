#ifdef __APP2__
#include "App.h"
struct VertexType
{
	DirectX::XMFLOAT3 Pos;	// 座標
	DirectX::XMFLOAT2 UV;	// 座標
};
bool WinApp::RenderInit() { return true; }
bool MyWinApp::RenderInit() {
	HRESULT hr = CreateWICTextureFromFile(
		GetD3D().GetDevice(),
		L"E:\\Work\\photo\\yuk.png",
		nullptr,
		m_textureView.GetAddressOf()
	); 
	m_bitmap = m_d2d->LoadWICImageFromFile(L"E:\\Work\\photo\\yuk.png");
    VertexType vertices[] = {
        {{-0.5, 0.5f, 0.0f},{0.0f,0.0f}},
        {{ 0.5, 0.5f, 0.0f},{1.0f,0.0f}},
        {{-0.5,-0.5f, 0.0f},{0.0f,1.0f}},
        {{ 0.5,-0.5f, 0.0f},{1.0f,1.0f}},
    };

    uint16_t indices[] = {
        0, 1, 2,
        2, 1, 3,
    };
    // 4. 정점 버퍼(VB) 생성 (Immutable: 수정 불가능, 빠름)
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(VertexType) * ARRAYSIZE(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE; // 데이터 안 바꿀거면 이게 제일 빠름

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    GetD3D().GetDevice()->CreateBuffer(&vbDesc, &vbData, m_quadVB.GetAddressOf());

    // 5. 인덱스 버퍼(IB) 생성
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(uint16_t) * ARRAYSIZE(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    GetD3D().GetDevice()->CreateBuffer(&ibDesc, &ibData, m_quadIB.GetAddressOf());

    // 6. 샘플러 상태 생성 (텍스처를 어떻게 확대/축소할지 정의)
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 부드럽게 보간
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    GetD3D().GetDevice()->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

}
#endif