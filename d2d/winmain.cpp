/*
I hate static_case 
*/
//#include <wrl.h>
#include "d2d.hpp"
#include <chrono>
#include <string>
D2D* g_d2d;
D3D* g_d3d;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
struct ShapeData{
	//char input_buffer[128];
	D2D1::ColorF::Enum color;
	float w, h;
	float x=0.f, y=0.f;
	float rot=0.f,drot=0.f;
	float alpha;
	ShapeData()
		: color(D2D1::ColorF::WhiteSmoke),
		w(100.0f),h(100.f),
		drot(0.0f),
		alpha(1.0f) {}
	void Reset() {
		memset(this, 0, sizeof(ShapeData));
		this->alpha = 1.0f;
		w = h = 100.0f;
	}
	void RandomSet() {
		x = (float)(rand() % 800); //static_cast<float>(`~~) lol typeing so long and hard
		y = (float)(rand() % 600);
		alpha= (float)(rand() % 100);
		rot = (float)(rand() % 360);
		drot = (float)(rand() % 100) / 100.f;
	}
	void Draw(ID2D1DeviceContext* dc,SolidBrush brush) {
		rot += drot;
		float center_x=this->x+ this->w/2.f, 
			center_y= this->y+ this->h/2.f;
		D2D1_MATRIX_3X2_F mat_rot = D2D1::Matrix3x2F::Rotation(this->rot, D2D1::Point2F(this->x, this->y));
		dc->SetTransform(mat_rot);
		brush->SetColor(D2D1::ColorF(this->color,this->alpha));
		dc->FillRectangle(D2D1::RectF(this->x - this->w / 2.f, this->y - this->h / 2.f, this->x + this->w / 2.f, this->y + this->h / 2.f), brush);
		
		brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
		/*const wchar_t buf[64] = L"fuck가가";
		dc->DrawTextW(buf, 10, g_d2d->textFormat,
		D2D1::RectF(this->x, this->y, this->x + this->w, this->y + this->h), brush);*/
		dc->SetTransform(D2D1::Matrix3x2F::Identity());
	}
};
struct ShapeDataDOD {
	enum {
		MAX_SHAPE=150000
	};
	//char input_buffer[MAX_SHAPE][128];
	alignas(16) float W[MAX_SHAPE], H[MAX_SHAPE];
	alignas(16) float X[MAX_SHAPE], Y[MAX_SHAPE];
	alignas(16) float ROT[MAX_SHAPE], DROT[MAX_SHAPE];
	alignas(16) float ALPHA[MAX_SHAPE];
	uint32_t size;
	D2D1::ColorF::Enum color[MAX_SHAPE];
	D2D1_COLOR_F COL[MAX_SHAPE];
	D2D1_RECT_F RT[MAX_SHAPE];
	D2D1_MATRIX_3X2_F TRANSFORM[MAX_SHAPE];
	
	ShapeDataDOD() {
		memset(this, 0, sizeof(ShapeDataDOD));
	}
	void ResetAll(bool isZero = false) {
		size = 0u;
		if (isZero) memset(this, 0, sizeof(ShapeDataDOD));
	}
	void AppendRandom() {
		if (size == MAX_SHAPE)return;
		W[size] = (float)(rand() % 100)+100; //static_cast<float>(`~~) lol typeing so long and hard
		H[size] = (float)(rand() % 100)+100;
		X[size] = (float)(rand() % 800); //static_cast<float>(`~~) lol typeing so long and hard
		Y[size] = (float)(rand() % 600);
		ALPHA[size] = (float)(rand() % 100);
		ROT[size] = (float)(rand() % 360);
		DROT[size] = (float)(rand() % 100) / 100.f;
		color[size] = (rand()>RAND_MAX/2)?D2D1::ColorF::Aqua:D2D1::ColorF::IndianRed;
		size++;
	}
	void Draw(ID2D1DeviceContext* dc, SolidBrush brush) {
		for (int i = 0; i < this->size; i++) {
			
			brush->SetColor(D2D1::ColorF(this->color[i], this->ALPHA[i]));
			dc->FillRectangle(
				D2D1::RectF(
					this->X[i] - this->W[i] / 2.f,
					this->Y[i] - this->H[i] / 2.f,
					this->X[i] + this->W[i] / 2.f, 
					this->Y[i] + this->H[i] / 2.f
				), 
				brush);
			//brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			/*const wchar_t buf[64] = L"fuck가가";
			dc->DrawTextW(buf, 10, g_d2d->textFormat,
				D2D1::RectF(this->X[i], this->Y[i], this->X[i] + this->W[i], this->Y[i] + this->H[i]),
				brush);*/
			dc->SetTransform(D2D1::Matrix3x2F::Identity());
		}
	}

	void Update() {
#pragma omp parallel for
		for (int i = 0; i < this->size; i++) {
			ROT[i] += DROT[i];
			RT[i] = D2D1::RectF(-W[i] / 2.f, -H[i] / 2.f, W[i] / 2.f, H[i] / 2.f);
			COL[i] = D2D1::ColorF(color[i], ALPHA[i]);
			TRANSFORM[i]= D2D1::Matrix3x2F::Rotation(this->ROT[i])
				* D2D1::Matrix3x2F::Translation(X[i]+W[i]/2.f, this->Y[i] + H[i] / 2.f);
		}
	}
	void DrawFast(ID2D1DeviceContext3* dc, ID2D1SpriteBatch* spriteBatch, ID2D1Bitmap* bitmap) {
		
		if (this->size == 0)return;
		D2D1_ANTIALIAS_MODE oldmode = dc->GetAntialiasMode();
		dc->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		spriteBatch->Clear();
		//HRESULT hr
		spriteBatch->AddSprites(
			this->size,
			this->RT,
			nullptr,
			COL,
			TRANSFORM,
			sizeof(D2D1_RECT_F),
			0,
			sizeof(D2D1_COLOR_F),
			sizeof(D2D1_MATRIX_3X2_F)
		);
		if (bitmap != nullptr)
			dc->DrawSpriteBatch(
				spriteBatch,
				bitmap,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				D2D1_SPRITE_OPTIONS_NONE
			);
		dc->SetAntialiasMode(oldmode);
	}
};
/*class DrawObject {
public:
	void UpdateCache(ID2D1DeviceContext* dc) {
		if (!m_isChange)return;
		ComPtr<ID2D1Bitmap>bitmap;
		dc->DrawBitmap(bitmap.Get(),);
		dc->DrawBitmap(bitmap)
	}
private:
	ComPtr<ID2D1BitmapRenderTarget> m_cacheBitmap;
	bool m_isChange = true;
};*/
static int shapecounts;
static int shapecounts2;
void RenderFrame() {
	float clearColor[4] = { 0.1f,0.1f,0.1f,1.0f };
	static std::vector<ShapeData>shapes;
	static ShapeDataDOD shapedod;
	static ShapeData s;
	static ID2D1GeometrySink* sink = nullptr;
	static ID2D1PathGeometry* path = nullptr;
	static bool isStart = false;
	static bool isStart_Normal = false;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin("Window B");
	// Add UI elements for the second window here
	ImGui::Text("This is the second window. DOD Style");
	if (ImGui::Button("Normal Benchmark start")) {
		if (isStart_Normal == false)shapes.clear(); 
		
		isStart_Normal = !isStart_Normal;
		for (int i = 0; i < 90000; i++) {
			s.RandomSet();
			shapes.emplace_back(s);
			shapecounts = shapes.size();
			s.Reset();
		}
	}
	if (ImGui::Button("DOD Benchmark start")) {
		if (isStart == false)shapedod.ResetAll();
		isStart = !isStart;
		for (int i = 0; i < 97000; i++) {
			shapedod.AppendRandom();
		}
	}
	if (ImGui::Button("Reset")) {
		shapedod.ResetAll();
		shapes.clear();
	}
	if (isStart) {
		shapedod.Update();
		shapedod.AppendRandom();
		shapecounts2 = shapedod.size;
	}
	if (isStart_Normal) {
		s.RandomSet();
		shapes.emplace_back(s);
		shapecounts = shapes.size();
		s.Reset();
	}
	ImGui::End();

	ImGui::Begin("ImGUI window");

	ImGui::Text("Hello");
	ImGui::SliderFloat("width", &s.w, 0.f, 800.0f);
	ImGui::SliderFloat("height", &s.h, 0.f, 600.0f);
	ImGui::SliderFloat("pos x", &s.x, 0.f, 800.0f);
	ImGui::SliderFloat("pos y", &s.y, 0.f, 600.0f);
	ImGui::SliderFloat("rotation", &s.drot, 0.f, 1.0f);
	ImGui::SliderFloat("alpha", &s.alpha, 0.f, 1.0f);
	//ImGui::InputText("Input Label", s.input_buffer, IM_ARRAYSIZE(s.input_buffer));

	static const char* items[] = {"red","blue","aqua"};
	static int current_item_idx = 0;
	ImGui::Combo("Shape color", &current_item_idx, items, IM_ARRAYSIZE(items));
	//s.ToUniText();
	switch (current_item_idx) {
	case 0:
		s.color = D2D1::ColorF::IndianRed;
		break;
	case 1:
		s.color = D2D1::ColorF::CadetBlue;
		break;
	case 2:
		s.color = D2D1::ColorF::Aquamarine;
		break;
	}
	if (ImGui::Button("Append")) {
		//isStart_Normal = !isStart_Normal;
	}
	
	if (ImGui::Button("Geometry") && path == nullptr) {
		g_d2d->factory->CreatePathGeometry(&path);
		path->Open(&sink);
		sink->SetFillMode(D2D1_FILL_MODE_WINDING);
		sink->BeginFigure(D2D1::Point2F(100, 100), D2D1_FIGURE_BEGIN_FILLED);
		D2D1_POINT_2F points[] = { D2D1::Point2F(0,0),D2D1::Point2F(100,0),D2D1::Point2F(0,200) };

		sink->AddLines(points, ARRAYSIZE(points));
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();
	}

	ImGui::End();


	g_d3d->m_deviceContext->ClearRenderTargetView(
		g_d3d->m_rtview.Get(), clearColor
	);
	g_d2d->BeginDraw();
	g_d2d->Clear(D2D1::ColorF(D2D1::ColorF::White));

	ID2D1DeviceContext* dc = g_d2d->deviceContext.Get();
	SolidBrush brush = g_d2d->solidBrush;
	for (auto &_shape : shapes) {
		_shape.Draw(dc, brush);
	}
	s.Draw(dc, brush);

	shapedod.DrawFast(g_d2d->dc3.Get(), g_d2d->spriteBatch.Get(), g_d2d->bakedBitmap.Get());

	if (path != nullptr) {
		brush->SetColor(D2D1::ColorF(1.0, 0.5, 1.0, 0.5));
		dc->FillGeometry(path, brush);
	}

	g_d2d->EndDraw();
	g_d3d->m_deviceContext->OMSetRenderTargets(1, g_d3d->m_rtview.GetAddressOf(), nullptr);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());//
	g_d3d->m_swapChain->Present(1, 0);

}
void ResizeApp(HWND hwnd,int width, int height) {
	if (!g_d3d || !g_d2d) return;

	// 1. 꼬리 자르기: 백버퍼를 참조하는 모든 객체 해제
	// (이거 안 하고 ResizeBuffers 하면 에러 뜹니다)
	g_d2d->deviceContext->SetTarget(nullptr); // D2D 문맥에서 타겟 떼기
	g_d2d->targetBitmap.Reset();        // 비트맵 날리기
	g_d3d->m_rtview.Reset();       // D3D RTV 날리기
	// 2. 스왑체인 크기 변경
	g_d3d->m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

	// 3. 다시 연결 (Init 코드와 비슷)
	// 3-1. D3D RTV 재생성
	ComPtr<ID3D11Texture2D> backBuffer;
	g_d3d->m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	g_d3d->m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &g_d3d->m_rtview);

	// 3-2. D2D 비트맵 재생성
	ComPtr<IDXGISurface> dxgiSurface;
	g_d3d->m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));

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

	g_d2d->deviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bp, &g_d2d->targetBitmap);
	g_d2d->deviceContext->SetTarget(g_d2d->targetBitmap.Get());




	//화면 크기 맞춤
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_d3d->m_deviceContext->RSSetViewports(1, &vp);
}
void SetupImGuiStyle()
{
	// Microsoft style by usernameiwantedwasalreadytaken from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(4.0f, 6.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(8.0f, 6.0f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(8.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 20.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 0.0f;
	//style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9490196f, 0.9490196f, 0.9490196f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.9490196f, 0.9490196f, 0.9490196f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 0.2f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15686275f, 0.28627452f, 0.47843137f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6862745f, 0.6862745f, 0.6862745f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6862745f, 0.6862745f, 0.6862745f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 0.2f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 0.2f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.2f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.1764706f, 0.34901962f, 0.5764706f, 0.862f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.19607843f, 0.40784314f, 0.6784314f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

	const int screenWidth = 800;
	const int screenHeight = 600;

	WNDCLASS wc = { 0 };
	HWND hwnd;

	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"D2DWindow";
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	RegisterClass(&wc);

	hwnd = CreateWindowExW(0,L"D2DWindow", L"D2D Class Example", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 
		screenWidth, screenHeight, nullptr, nullptr, hInstance, nullptr);

	// D3D11 Device 생성
	g_d3d = new D3D();
	if (!g_d3d->Init(hwnd, screenWidth, screenHeight)) {
		MessageBox(hwnd, L"D3D 초기화 실패!", L"Error", MB_ICONERROR);
		return 0;
	}
	// D2D 초기화
	g_d2d = new D2D();
	if (!g_d2d->Init(hwnd, g_d3d->m_device,g_d3d->m_swapChain)) {
		MessageBox(hwnd, L"D2D 초기화 실패!", L"Error", MB_ICONERROR);
		return 0;
	}
	if (!g_d3d->InitImGui()) {
		MessageBox(hwnd, _T("ImGui Error"), _T("ImGui Error"), MB_ICONERROR);
		return 0;
	}
	SetupImGuiStyle();
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	std::chrono::steady_clock::time_point m_lastTime; // 마지막 프레임 시간
	float m_fps = 0.0f;
	int m_frameCount = 0;
	float m_timeElapsed = 0.0f;
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			auto currentTime = std::chrono::steady_clock::now();
			// 마지막 프레임과의 시간 차이 (초 단위)
			float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastTime).count() / 1000000.0f;
			m_lastTime = currentTime;

			// 1초마다 FPS 갱신 (더 부드러운 표시)
			m_timeElapsed += deltaTime;
			m_frameCount++;
			if (m_timeElapsed >= 1.0f) {
				m_fps = static_cast<float>(m_frameCount) / m_timeElapsed;
				m_frameCount = 0;
				m_timeElapsed = 0.0f;

				std::wstring fpsText = L"My DX11 App (FPS: " + std::to_wstring(m_fps);
				fpsText += L", CNT : ";
				fpsText += std::to_wstring(shapecounts) + L", CNT2 : ";
				fpsText += std::to_wstring(shapecounts2) + L")";
				// m_hwnd는 WinApp 클래스에 있는 윈도우 핸들입니다.
				SetWindowTextW(hwnd, fpsText.c_str());

			}
			RenderFrame();
		}
	}
	delete g_d2d;
	delete g_d3d;
	return (int)msg.wParam;
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_PAINT: {
		ValidateRect(hwnd, NULL);
		return 0;
	}
	case WM_SIZE:
		if (g_d3d && wParam != SIZE_MINIMIZED) {
			int newWidth = (int)LOWORD(lParam);
			int newHeight = (int)HIWORD(lParam);
			ResizeApp(hwnd, newWidth, newHeight);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}