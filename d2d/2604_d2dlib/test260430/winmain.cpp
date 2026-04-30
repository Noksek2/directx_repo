#include <d2d.hpp>
#include <App.hpp>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

class MyApp;
MyApp* g_app;

UINT32 gControlID = 0;

constexpr UINT32 WINCONTROL_ID_BEGIN = 0;
enum {
	BTN_1= WINCONTROL_ID_BEGIN, 
	BTN_2,
};
static const UINT SetWinColor(BYTE r, BYTE g, BYTE b, BYTE a) {
	return (a << 24) | (r << 16) | (r << 8) | (b);
}
struct WinControlData{
	enum {
		CT_Button,
		CT_Static,
	};
	HWND hControl;
	UINT CID;
	union {
		WNDPROC mWndProc;
		SUBCLASSPROC mSubProc;
	};
};
class IWinControl : public WinControlData {
public:
	virtual void OwnerDraw(LPDRAWITEMSTRUCT pDIS, ID2D1SolidColorBrush* m_brush) = 0;
};
class WinButtonControl : public IWinControl{
public:
	enum {
		BE_Hovered = 1 << 0,
		BE_Pressed = 1 << 1,
	};
	const TCHAR* mText;
	UINT mHoverColor;
	UINT mPressColor;
	UINT mButtonEvent;
	
	UINT32 CreateButton(HWND hwnd, const TCHAR* txt, int x,int y, int W, int H, SUBCLASSPROC subProc) {
		if (gControlID == MAXINT32) __debugbreak();
		CID = gControlID;
		hControl = CreateWindow(L"BUTTON", txt,
							   WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
							   x, y, W, H, hwnd, (HMENU)(CID), NULL, NULL);
		mText = txt;
		SetWindowSubclass(hControl, subProc, CID, 0);

		mPressColor = 0U;
		mHoverColor = 0U;
		mButtonEvent = 0U;
		return gControlID++;
	}
	void OwnerDraw(LPDRAWITEMSTRUCT pDIS, ID2D1SolidColorBrush* m_brush) override;
};



LRESULT CALLBACK ButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class MyApp : public App {
public:
	WinButtonControl* const GetButton(UINT ID) {
		return &mControls[ID];
	}
	ID2D1SolidColorBrush* const GetBrush() { return m_brush.Get(); }

	bool InitRender() override {

		auto mDC = m_d2d->mDeviceContext;
		auto mDCRT = m_d2d->mDCRT;
		m_d2d->CreateFontDevice(L"Consolas", 15);
		m_d2d->CreateFontDevice(L"Consolas", 20);

		HRESULT hr = mDCRT->CreateSolidColorBrush(D2D1::ColorF(0.5, 0.1, 0.2, 0.6), &m_brush);
		return (SUCCEEDED(hr));
	}

	void Render() override {
		D3D* d3d = m_d2d->mD3D;
		auto mDC = m_d2d->mDeviceContext;
		const float clearColor[4] = { 0.1f,0.1f,0.1f,1.0f };
		//ImGui_BeginRender();
		//ImGui::Begin("ImGui Debug");
		//ImGui::End();
		m_d2d->mD3D->mDeviceContext->ClearRenderTargetView(d3d->mRtView, clearColor);
		m_d2d->BeginDraw();
		m_d2d->Clear(D2D1::ColorF(D2D1::ColorF::White));

		mDC->FillRectangle(D2D1::RectF(50, 50, 150, 150), m_brush.Get());



		m_d2d->EndDraw();
		d3d->mDeviceContext->OMSetRenderTargets(1, &d3d->mRtView, nullptr);
		//ImGui_EndRender();
		d3d->mSwapChain->Present(1, 0);
	}
	void OnCreate(HWND hwnd) {

		mControls[0].CreateButton(hwnd, L"Button1", 10, 10, 100, 30, ButtonSubclassProc);
		mControls[0].mHoverColor = SetWinColor(150, 100, 200, 128);
		mControls[0].mPressColor = SetWinColor(150, 100, 200, 128);

		// 버튼 생성 후 호출
		//SetWindowTheme(mControls[0].hControl, L"", L"");

		mControls[1].CreateButton(hwnd, L"Button2", 80, 25, 70, 20, ButtonSubclassProc);
		mControls[1].mHoverColor = SetWinColor(150, 100, 200, 128);
		mControls[1].mPressColor = SetWinColor(150, 100, 200, 128);

		mControls[2].CreateButton(hwnd, L"Button3", 30, 75, 70, 20, ButtonSubclassProc);
		mControls[2].mHoverColor = SetWinColor(150, 100, 200, 128);
		mControls[2].mPressColor = SetWinColor(150, 100, 200, 250);

		mControls[3].CreateButton(hwnd, L"Button4", 30, 125, 70, 20, ButtonSubclassProc);
		mControls[3].mHoverColor = SetWinColor(150, 100, 200, 128);
		mControls[3].mPressColor = SetWinColor(150, 100, 200, 250);


		mControls_cnt = 4;
	}
	void OnPaint() {

	}
	void OnCommand(WPARAM wParam) {
		switch (LOWORD(wParam)) {
		case BTN_1:
			MessageBox(m_hwnd, L"Changed", mControls[0].mText, MB_OK);
			mControls[0].mText = L"Chit";
			InvalidateRect(mControls[0].hControl, NULL, FALSE);
			break;
		case BTN_2:
			MessageBox(m_hwnd, L"Changed 2", mControls[1].mText, MB_OK);
			mControls[1].mText = L"Chit2";
			InvalidateRect(mControls[1].hControl, NULL, FALSE);
			break;
		}
	}
	void OnDrawItem(LPARAM lParam) {
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
		UINT CID = pDIS->CtlID;
		D2D* d2d = m_d2d;
		for (int i = 0; i < mControls_cnt; i++) {
			if (mControls[i].CID == CID)
				mControls[i].OwnerDraw(pDIS, m_brush.Get());
		}
		if (CID == BTN_1 || CID == BTN_2) { // 버튼 ID 확인
			
			/*HBRUSH hBrush = (state & ODS_SELECTED) ?
				CreateSolidBrush(RGB(200, 0, 0)) : CreateSolidBrush(RGB(0, 120, 215));

			// 배경 그리기
			FillRect(hdc, &rect, hBrush);
			DeleteObject(hBrush);

			// 텍스트 그리기
			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkMode(hdc, TRANSPARENT);
			DrawText(hdc, L"GDI Button", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			*/
		}
	}

protected:
	WinButtonControl mControls[10];
	int mControls_cnt = 0;

	ComPtr<ID2D1SolidColorBrush> m_brush;
};

int WINAPI WinMain(HINSTANCE hin, HINSTANCE, LPSTR, int nCmdShow) {

	const int screenWidth = 800;
	const int screenHeight = 600;

	const TCHAR* className = _T("ImageCompress_Win32");
	const TCHAR* title = _T("ImageCompress Win32");
	MSG msg = { 0 };
	g_app = new MyApp();
	if (!g_app->Create(hin, WndProc, className, title, screenWidth, screenHeight)) {
		//MessageBox(0,L"D2D Error",L"",MB_ICONERR)
		goto lEnd;
	}
	if (!g_app->InitRender()) {
		MessageBox(0, L"D2D InitRender Error", L"", MB_ICONERROR);
		goto lEnd;
	}
	g_app->Show();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//g_app->GetTick();
			//g_app->Render();
		}
	}
lEnd:
	delete g_app;
	return (int)msg.wParam;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	//if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;

	switch (msg) {
	case WM_CREATE: {
		g_app->OnCreate(hwnd);
		return 0;
	}
	case WM_DRAWITEM:
	{
		g_app->OnDrawItem(lParam);
		return 0;
	}
	case WM_COMMAND:
	{
		g_app->OnCommand(wParam);
		return 0;
	}
	case WM_PRINTCLIENT:
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = (msg==WM_PAINT)?BeginPaint(hwnd, &ps):(HDC)wParam;
		HBRUSH hbrush= CreateSolidBrush(RGB(150, 120, 200));
		HGDIOBJ hprev = SelectObject(hdc, hbrush);
		Ellipse(hdc, 10, 10, 100, 100);
		DeleteObject(hbrush);
		SelectObject(hdc, hprev);
		if (msg == WM_PAINT)EndPaint(hwnd, &ps);

		//ValidateRect(hwnd, NULL);
		return 0;
	}
	case WM_SIZE:
		if (g_app && wParam != SIZE_MINIMIZED) {
			int newWidth = (int)LOWORD(lParam);
			int newHeight = (int)HIWORD(lParam);
			g_app->ResizeScreen(newWidth, newHeight);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void WinButtonControl::OwnerDraw(LPDRAWITEMSTRUCT pDIS, ID2D1SolidColorBrush* m_brush) {


	D2D* d2d = g_app->GetD2D();
	HDC hdc = pDIS->hDC;
	RECT rect = pDIS->rcItem;

	// 상태 확인 (눌림, 포커스 등)
	UINT state = pDIS->itemState;
	HWND hParent = GetParent(pDIS->hwndItem);

	


	SendMessage((pDIS->hwndItem), WM_ERASEBKGND, (WPARAM)hdc, 0);
	
	// 1. 부모의 배경을 버튼 DC로 복사 (좌표 보정 필요)
	POINT pt = {0, 0};
	// 버튼의 (0,0) 좌표가 부모 기준으로는 어디인지 계산
	MapWindowPoints(pDIS->hwndItem, GetParent(pDIS->hwndItem), &pt, 1);

	// DC의 원점을 부모 기준으로 옮겨서 부모가 "내 위치"에 그리게 함
	POINT oldOrg;
	OffsetWindowOrgEx(hdc, pt.x, pt.y, &oldOrg);

	// ★ 대상은 반드시 GetParent(...)여야 함
	SendMessage(hParent, WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT);

	// 원상 복구
	SetWindowOrgEx(hdc, oldOrg.x, oldOrg.y, NULL);

	
	//SendMessage(GetParent(pDIS->hwndItem), WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT);
	d2d->mDCRT->BindDC(hdc, &rect);
	d2d->mDCRT->BeginDraw();
	d2d->mDCRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

	// 2. WPF 느낌의 둥근 사각형 그리기
	D2D1_RECT_F rt = D2D1::RectF(rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2);
	D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
		rt,
		5.0f, 5.0f);
	D2D1_RECT_F layoutRect = D2D1::RectF(
		0, 0,
		(float)(rect.right - rect.left),
		(float)(rect.bottom - rect.top)
	);
	const TCHAR* text = mText;
	ID2D1Brush* b = m_brush;
	
	if ((state & ODS_SELECTED)) {
		mButtonEvent = BE_Pressed;
		m_brush->SetColor(D2D1::ColorF(mPressColor & 0xFFFFFF, (float)(mPressColor >> 24)/255.f));
	}
	else if (mButtonEvent & BE_Hovered) {
		m_brush->SetColor(D2D1::ColorF(mHoverColor & 0xFFFFFF, (float)(mHoverColor >> 24) / 255.f));
	}
	else {
		mButtonEvent &= ~(0b11U);
		m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
	}
	d2d->mDCRT->FillRoundedRectangle(roundedRect, m_brush);
	//d2d->mDCRT->FillRectangle(rt, m_brush);
	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black,0.5f));
	d2d->mDCRT->DrawRoundedRectangle(roundedRect, m_brush,0.3f);

	//d2d->mDCRT->DrawRectangle(rt, m_brush);
	//m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

	d2d->mDCRT->DrawTextW(
		text,
		(UINT32)wcslen(text),
		d2d->mDWTextFormat[0],
		layoutRect,
		m_brush
	);

	d2d->mDCRT->EndDraw();
	//SetBkMode(hdc, TRANSPARENT);
}

LRESULT CALLBACK ButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	auto Btn=g_app->GetButton(uIdSubclass);
	switch (uMsg) {
	case WM_PAINT: {
		//D2D* d2d = g_app->GetD2D();
		//PAINTSTRUCT ps;
		//HDC hdc = BeginPaint(hWnd, &ps);
		//RECT rect;
		//GetClientRect(hWnd, &rect);
		//
		//// 부모에게 현재 내 HDC에 그려달라고 요청
		//SendMessage((hWnd), WM_ERASEBKGND, (WPARAM)hdc, 0);
		//SendMessage(GetParent(hWnd), WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT);
		//
		//// 2. Direct2D 렌더링
		//d2d->mDCRT->BindDC(hdc, &rect);
		//d2d->mDCRT->BeginDraw();
		////d2d->mDCRT->Clear(D2D1::ColorF(0.1, 0.1, 0.1, 1.0f));
		//
		//// 여기서 Clear(0, 0) 하면 아까 땡겨온 부모 배경이 날아가니 절대 금지
		//// 바로 둥근 버튼 그리기
		//auto mBrush=g_app->GetBrush();
		//mBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue, 0.2f));
		//d2d->mDCRT->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(0, 0, (float)rect.right, (float)rect.bottom), 10, 10), mBrush);
		//
		//d2d->mDCRT->EndDraw();
		//
		//EndPaint(hWnd, &ps);

	}break;
	case WM_MOUSEMOVE:
	{
		auto isHover = Btn->mButtonEvent & WinButtonControl::BE_Hovered;
		if (!isHover) {
			Btn->mButtonEvent |= WinButtonControl::BE_Hovered;

			// 마우스가 나가는 것을 감지하도록 예약
			TRACKMOUSEEVENT tme = { sizeof(tme) };
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);

			// 호버 상태가 되었으니 다시 그리라고 명령
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	}
	case WM_ERASEBKGND:
		// 0이 아닌 값을 반환하면 시스템은 배경을 다 지운 것으로 간주하고 아무것도 안 합니다.
		return 1;
	case WM_MOUSELEAVE:
	{
		auto isHover = Btn->mButtonEvent & WinButtonControl::BE_Hovered;
		Btn->mButtonEvent &= ~WinButtonControl::BE_Hovered;
		// 마우스가 나갔으니 다시 그리라고 명령
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_NCDESTROY:
		// 윈도우가 파괴될 때 서브클래싱 해제
		RemoveWindowSubclass(hWnd, ButtonSubclassProc, uIdSubclass);
		break;

	}
	//현재 처리하지 못한 메세지 (원래 버튼 로직, 메세지)는 돌려보냄
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

