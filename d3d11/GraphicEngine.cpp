#include "GraphicEngine.hpp"

GraphicEngine::GraphicEngine() {

}
GraphicEngine::~GraphicEngine() {

}
bool GraphicEngine::Init(HWND hwnd, int width, int height) {
	if (!m_d3d.Init(hwnd, width, height)) {
		MessageBoxW(hwnd, L"D3D11 Init failed", L"Error", MB_OK);
		return false;
	}
	if (!m_d2d.Init(hwnd, m_d3d.GetSwapChain().Get(), m_d3d.GetDevice().Get())) {
		MessageBoxW(hwnd, L"D2D Init failed", L"Error", MB_OK);
		return false;
	}
	if (!m_d3d.InitImGui()) {
		MessageBoxW(hwnd, L"Imgui Init failed", L"Error", MB_OK);
		return false;
	}
	return true;
}
