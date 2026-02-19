#define __APP1__
#ifdef __APP1__
#include "App.h"
#include "ImGuizmo.h"
struct VertexType
{
	DirectX::XMFLOAT3 Pos;	// 座標
};
struct Vertex {
	float x, y, z;
	float r, g, b, a;
};

bool WinApp::RenderInit() { return true; }
bool MyWinApp::RenderInit() {
	srand(time(0));
	this->f = 0.f;
	this->color = 0.f;
	this->m_pos = { 0,0 };


	constexpr int vertex_len = 8;
	constexpr int indices_len = 36;
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// デバイスにバインドするときの種類(頂点バッファ、インデックスバッファ、定数バッファなど)
	vbDesc.ByteWidth = sizeof(Vertex)* vertex_len;					// 作成するバッファのバイトサイズ
	vbDesc.MiscFlags = 0;							// その他のフラグ
	vbDesc.StructureByteStride = 0;					// 構造化バッファの場合、その構造体のサイズ
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;				// 作成するバッファの使用法
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;;

	// 上の仕様を渡して頂点バッファを作ってもらう
	// もちろんデバイスさんにお願いする
	// 頂点バッファの作成
	GetD3D().GetDevice()->CreateBuffer(&vbDesc, nullptr, &m_vb);

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 인덱스 버퍼용
	ibDesc.ByteWidth = sizeof(unsigned short) * 36; // 삼각형 12개 * 점 3개 = 36개
	ibDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetD3D().GetDevice()->CreateBuffer(&ibDesc, nullptr, &m_ib);


#ifdef USED2D
	m_bitmap=m_d2d->LoadWICImageFromFile(L"E:\\Work\\photo\\yuk.png");
#else
	// 0. 준비 (로딩 시 한 번)
	// ... 텍스처 로딩 (예: WIC 로더 사용) ...
	CreateWICTextureFromFile(GetD3D().GetDevice(), L"E:\\Work\\photo\\yuk.png",
		nullptr, m_texture.GetAddressOf());

	// --- 렌더링 루프 끝 ---
#endif
	return true;
}
bool WinApp::Render() {

	//Time
	auto currentTime = std::chrono::steady_clock::now();
	// 마지막 프레임과의 시간 차이 (초 단위)
	float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastTime).count() / 1000000.0f;
	m_deltaTime = deltaTime;
	m_lastTime = currentTime;

	// 1초마다 FPS 갱신 (더 부드러운 표시)
	m_timeElapsed += deltaTime;
	m_frameCount++;
	if (m_timeElapsed >= 1.0f) {
		m_fps = static_cast<float>(m_frameCount) / m_timeElapsed;
		m_frameCount = 0;
		m_timeElapsed = 0.0f;

		// ★★★ 여기에 FPS를 출력하는 코드가 들어갑니다 ★★★
		// (아래 2. 텍스트 출력 방법 참고)
		std::wstring fpsText = L"My DX11 App (FPS: " + std::to_wstring(m_fps) + L")";
		// m_hwnd는 WinApp 클래스에 있는 윈도우 핸들입니다.
		SetWindowTextW(m_hwnd, fpsText.c_str());
	}
	return true;
}
bool MyWinApp::Render() {
#ifdef USEIMGUI

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
#endif
	auto context = GetD3D().GetDeviceContext();

	// 매 프레임마다 깊이 값 초기화 (1.0f는 가장 먼 곳을 의미)
	context->ClearDepthStencilView(GetD3D().m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// ImGuizmo는 float[16] 배열 형식을 씁니다.
// DirectXMath 행렬에서 데이터를 뽑아냅니다.
	ConstantBuffer cb;
	float viewArr[16], projArr[16], worldArr[16];

	
	// 조작된 결과를 다시 사용자님의 World 행렬로 덮어쓰기
	//m_worldMatrix = XMLoadFloat4x4((XMFLOAT4X4*)worldArr);

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		m_pos.x -= 1.f;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		m_pos.x += 100.f * m_deltaTime;
	}
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
		SendMessage(m_hwnd, WM_DESTROY, 0, 0);
		return false;
	}
	{
		constexpr int vertex_len = 8;
		constexpr int indices_len = 36;
		// 1. 큐브 데이터 준비 (사용자님이 주신 데이터)
		Vertex v[vertex_len] = {
			{ -1.0f,  1.0f, -1.0f, 1, 0, 0, 1 }, // 0
			{  1.0f,  1.0f, -1.0f, 0, 1, 0, 1 }, // 1
			{  1.0f, -1.0f, -1.0f, 0, 0, 1, 1 }, // 2
			{ -1.0f, -1.0f, -1.0f, 1, 1, 0, 1 }, // 3
			{ -1.0f,  1.0f,  1.0f, 1, 0, 1, 1 }, // 4
			{  1.0f,  1.0f,  1.0f, 0, 1, 1, 1 }, // 5
			{  1.0f, -1.0f,  1.0f, 1, 1, 1, 1 }, // 6
			{ -1.0f, -1.0f,  1.0f, 0, 0, 0, 1 }  // 7
		};

		unsigned short indices[indices_len] = {
			0, 1, 2,  0, 2, 3, // 앞
			5, 4, 6,  4, 7, 6, // 뒤
			4, 0, 3,  4, 3, 7, // 좌
			1, 5, 6,  1, 6, 2, // 우
			4, 5, 1,  4, 1, 0, // 상
			3, 2, 6,  3, 6, 7  // 하
		};

		
		cb.mWorld = XMMatrixIdentity();
		float t = static_cast<float>(GetTickCount()) * 0.001f;

		// 카메라가 원점을 중심으로 뱅글뱅글 돕니다
		XMVECTOR Eye ;
		XMVECTOR At  ;
		XMVECTOR Up  ;
		
		switch (m_rotType) {
		case 0:
			Eye = XMVectorSet(5.0f * sinf(t), 3.0f, -5.0f * cosf(t), 0.0f);
			At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  // 바라보는 곳 (원점)
			Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  // 머리 위 방향
			break;
		case 1:
			Eye = XMVectorSet(5.0f, 5.0f * sinf(t), -5.0f * cosf(t), 0.0f);
			At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  // 바라보는 곳 (원점)
			Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  // 머리 위 방향
			break;
		default:
			static float cameraX = 0.0f, cameraY = 3.0f, cameraZ = -5.0f;
			float speed = 0.5f;

			// 2. 키 입력 체크 (GetAsyncKeyState 사용)
			if (GetAsyncKeyState('W') & 0x8000) cameraZ += speed;
			if (GetAsyncKeyState('S') & 0x8000) cameraZ -= speed;
			if (GetAsyncKeyState('A') & 0x8000) cameraX -= speed;
			if (GetAsyncKeyState('D') & 0x8000) cameraX += speed;
			
			Eye = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
			At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		}
		cb.mView = XMMatrixTranspose(XMMatrixLookAtLH(Eye, At, Up)); // DX11은 전치 행렬로 넘겨야 함

		// 3. Projection: 원근감 설정 (화각 45도)
		float aspectRatio = 1280.0f / 720.0f;
		cb.mProjection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.01f, 100.0f));

		// --- [GPU로 전송] ---
		if (GetD3D().m_constantBuffer) {
			context->UpdateSubresource(GetD3D().m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
			context->VSSetConstantBuffers(0, 1, GetD3D().m_constantBuffer.GetAddressOf());
		}


		// 2. 정점 버퍼 Map/Unmap
		D3D11_MAPPED_SUBRESOURCE mappedV;
		context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedV);
		memcpy(mappedV.pData, v, sizeof(v));
		context->Unmap(m_vb.Get(), 0);

		// 3. 인덱스 버퍼 Map/Unmap (추가!)
		D3D11_MAPPED_SUBRESOURCE mappedI;
		context->Map(m_ib.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedI);
		memcpy(mappedI.pData, indices, sizeof(indices));
		context->Unmap(m_ib.Get(), 0);

		// 4. 파이프라인 설정
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);

		// 중요: 인덱스 버퍼 연결!
		context->IASetIndexBuffer(m_ib.Get(), DXGI_FORMAT_R16_UINT, 0);

		// 중요: STRIP이 아니라 LIST로 바꿔야 인덱스 순서대로 그려집니다.
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->VSSetShader(GetD3D().m_VS.Get(), 0, 0);
		context->PSSetShader(GetD3D().m_PS.Get(), 0, 0);
		context->IASetInputLayout(GetD3D().m_InputLayout.Get());

		// 5. 그리기 (Draw 대신 DrawIndexed 사용)
		// 인덱스 36개를 써서 그려라!
		context->DrawIndexed(indices_len, 0, 0);
	}
	{

		// 일단 이 쪽은 나중에 이해하기로

		// 三角形を作るため、頂点を３つ作る
		VertexType v[3] = {
			{{0,0,0}},
			{{1,-f,0}},
			{{-1,-1,0}},
		};

		//-----------------------------
		// 頂点バッファ作成
		// ・上記で作った３つの頂点はそのままでは描画に使用できないんす…
		// ・ビデオメモリ側に「頂点バッファ」という形で作る必要があります！
		// ・今回は効率無視して、その場で作って、使って、すぐ捨てます。
		//-----------------------------
		// 作成するバッファの仕様を決める
		// ・今回は頂点バッファにするぞ！って感じの設定
		
		D3D11_MAPPED_SUBRESOURCE mappedData;
		context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		memcpy(mappedData.pData, v, sizeof(v)); // 
		context->Unmap(m_vb.Get(), 0);

		// 
		UINT stride = sizeof(VertexType);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// 
		context->VSSetShader(GetD3D().m_VS.Get(), 0, 0);
		context->PSSetShader(GetD3D().m_PS.Get(), 0, 0);
		context->IASetInputLayout(GetD3D().m_InputLayout.Get());

		// 
		context->Draw(3, 0);
	}


	WinApp::Render();
	// バックバッファの内容を画面に表示
		// 문자열 포맷팅
	wchar_t fpsString[32];
	swprintf_s(fpsString, L"FPS: %.2f %.2f", m_fps,m_pos.x); // m_fps는 위에서 계산한 값
	

#ifdef USED2D
	if(m_bD2D){
		auto rt = m_d2d->GetDeviceContext();
		auto brush = m_d2d->GetSolidBrush();
		D2D1_RECT_F rect = { 0,0,200,200 };
		//rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		//rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
		rt->BeginDraw();

		if (!this->m_bWinRatioFixed) {
			float scaleX = (float)m_currentWidth / (float)m_screenWidth;
			float scaleY = (float)m_currentHeight / (float)m_screenHeight;
			D2D1::Matrix3x2F scaleMatrix = Matrix3x2F::Scale(scaleX, scaleY);
			rt->SetTransform(&scaleMatrix);
		}
		//rt->Clear(ColorF(ColorF::White));
		rt->DrawBitmap(m_bitmap, RectF(200, 200, 800, 850));
		rt->DrawBitmap(m_bitmap, RectF(100+m_pos.x, 100, 500 + m_pos.x, 550));
		rt->DrawBitmap(m_bitmap, rect);
		brush->SetColor(ColorF(color, 0.f, 0.f));
		rt->FillEllipse(Ellipse(Point2F(200.f, 180.f), 100.f, 80.f), brush);
		rt->SetTransform(D2D1::Matrix3x2F::Identity());
		rt->EndDraw();
	}

#else
	// 2D 그리기 시작
	SpriteBatch* m_spriteBatch = GetD3D().GetSpriteBatch();
	SpriteFont* m_spriteFont = GetD3D().GetSpriteFont();
	// --- 렌더링 루프 ---
	DirectX::SimpleMath::Matrix scaleMatrix;
	if (!this->m_bWinRatioFixed) {
		float scaleX = (float)m_currentWidth / (float)m_screenWidth;
		float scaleY = (float)m_currentHeight / (float)m_screenHeight;
		 scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(scaleX, scaleY, 1.0f);
		// Begin()의 6번째 인자로 스케일 행렬을 넘겨줍니다.
		
		// ▲▲▲ ▲▲▲
	}
	else {
		float offsetX = 0.f;//((float)m_currentWidth - (float)m_screenWidth) / 2.0f;
		float offsetY = 0.f; ((float)m_currentHeight - (float)m_screenHeight) / 2.0f;
		
		scaleMatrix = DirectX::SimpleMath::Matrix::CreateTranslation(offsetX, offsetY, 0.0f);
	}
	m_spriteBatch->Begin(SpriteSortMode_Deferred,
		nullptr, nullptr, nullptr,
		GetD3D().GetRasterState(),
		nullptr,
		scaleMatrix);
	m_spriteBatch->Draw(m_texture.Get(), XMFLOAT2(100, 100));
	
	float rotation = XM_PI / 180.0 * (f*180); // 45도 (PI / 4)
	m_spriteBatch->Draw(m_texture.Get(), XMFLOAT2(m_pos.x, 300), // 화면 위치
		nullptr, Colors::White, rotation, XMFLOAT2(100, 100));

	// 3. 기울이기 (Shear) - DXTK는 '기울이기'를 직접 지원하지 않습니다.
	//    대신 SpriteBatch에 사용자 지정 변환 행렬(Transform Matrix)을 넘겨줘야 합니다.
	//    이건 조금 고급 기법입니다.

	// 폰트 그리기
	GetD3D().GetSpriteFont()->DrawString(
		GetD3D().GetSpriteBatch(), // 사용할 배치
		fpsString,           // 출력할 문자열
		DirectX::XMFLOAT2(20.f, 10.f),
		DirectX::Colors::Aqua
		//DirectX::SimpleMath::Vector2(10.0f, 10.0f) // 화면 좌표 (X, Y)
		// , DirectX::Colors::White // 색상 (기본값 White)
	);

	// 2D 그리기 종료
	m_spriteBatch->End();
#endif

#ifdef USEIMGUI

	ImGuizmo::SetID(0);
	// 주의: 전치(Transpose)하지 않은 원본 행렬을 넘겨야 계산이 맞습니다!
	XMStoreFloat4x4((XMFLOAT4X4*)viewArr, cb.mView);
	XMStoreFloat4x4((XMFLOAT4X4*)projArr, cb.mProjection);
	XMStoreFloat4x4((XMFLOAT4X4*)worldArr, cb.mWorld); // 조작할 물체의 행렬

	// 화면 크기 설정
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	// 핸들 출력 및 조작 (Translate, Rotate, Scale 중 선택)
	ImGuizmo::Manipulate(viewArr, projArr, ImGuizmo::ROTATE, ImGuizmo::LOCAL, worldArr);

	// 2. ImGui UI 코드 작성 (여기서부터 자유롭게!)
	//    (가장 쉬운 테스트: 데모 창 띄우기)
	
	// "내 컨트롤 패널"이라는 이름의 창을 하나 띄운다.
	ImGui::Begin("My Control Panel");

	// (1) 3D 삼각형의 위치(f)를 슬라이더로 조작
	// MyWinApp 클래스의 멤버 변수 f를 직접 제어합니다.
	ImGui::SliderFloat("Triangle Y Pos", &this->f, -1.0f, 1.0f);
	ImGui::SliderFloat("Red", &this->color, 0.0f, 1.0f);
	ImGui::SliderFloat("DEF", &this->m_def, -1000.0f, 1000.0f);
	ImGui::SliderInt("Type", &this->m_rotType, 0, 10);
	if (ImGui::Button("Reset Position"))
	{
		this->f = 0.0f;
	}

	if (ImGui::Checkbox("Enable d2d", &m_bD2D))
	{
	}
	if (ImGui::Checkbox("Enable Fixed Window Mode", &m_bWinRatioFixed))
	{
		// [즉시 적용] 값이 변경된 순간 D3D 뷰포트를 즉시 재설정합니다.
		UpdateD3DViewport(m_currentWidth, m_currentHeight);
	}
	ImGui::End();

	// 3. ImGui 렌더링 (그리기)
	ImGui::Render(); // ImGui가 그릴 내용을 계산

	// ... (여기서 본인의 3D/2D 렌더링을 합니다) ...
	// ... (m_pImmediateContext->ClearRenderTargetView() 등) ...
	// ... (m_D2DDeviceContext->BeginDraw() ... EndDraw() 등) ...

	// 4. ImGui가 계산한 Draw Data를 DX11에 넘겨서 최종 렌더링
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// ... (마지막에 SwapChain->Present()) ...

#endif

	return true;



	// 三角形の描画

}
#endif