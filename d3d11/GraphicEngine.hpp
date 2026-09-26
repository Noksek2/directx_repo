#ifndef GRAPHICENGINE_HPP
#define GRAPHICENGINE_HPP

#include "D3D.hpp"
#include "D2D.hpp"

class GraphicEngine {
public:
	GraphicEngine();
	~GraphicEngine();

	bool Init(HWND hwnd, int width, int height);

	D3D& GetD3D() { return m_d3d; }
	D2D& GetD2D() { return m_d2d; }
	void BeginFrame(float r, float g, float b, float a) {
		float clearColor[] = { r,g,b,a };
		m_d3d.GetDeviceContext()->ClearRenderTargetView(
			m_d3d.GetRenderTargetView().Get(),
			clearColor
		);
		m_d2d.GetRenderTarget()->BeginDraw();
	}
	void EndFrame() {
		m_d2d.GetRenderTarget()->EndDraw();
		m_d3d.GetSwapChain()->Present(1, 0);
	}
private:
	D3D m_d3d;
	D2D m_d2d;
};

#endif