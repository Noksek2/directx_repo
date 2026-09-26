#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define USEIMGUI
#define USED2D

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <string>
#include <unordered_map>

#include <Windows.h>
#include <tchar.h>
#include <math.h>

#include <DirectXMath.h>

#include <wincodec.h>
#include <wrl/client.h>

#include <d3d11.h>
#include <d3dcompiler.h>


#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d2d1helper.h>
#include <dwrite.h>

//d3d11, dxgi.lib
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib, "d3dcompiler.lib")


#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "windowscodecs")

using namespace DirectX;
using namespace D2D1;

using Microsoft::WRL::ComPtr;

//#include <SpriteBatch.h>
//#include <SpriteFont.h>
//#include <SimpleMath.h>
//#include <WICTextureLoader.h>



#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"



#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
//#include "UserDef.h"
