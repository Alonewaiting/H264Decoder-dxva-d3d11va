//----------------------------------------------------------------------------------------------
// CDXRenderer.h
//----------------------------------------------------------------------------------------------
#ifndef CDXRenderer_H
#define CDXRenderer_H
#include "windows.h"
#include "Dxva2Definition.h"
#include "d3d9.h"
#include "d3d11.h"
#include "StdAfx.h"
class IDirect3DDeviceManager9;
class CDXRenderer{
public:
	
	// Dxva2Renderer.cpp
	virtual HRESULT InitDXVA2(const HWND, const UINT, const UINT, const UINT, const UINT, DXVA2_VideoDesc&, const MFTIME) = 0;
	virtual void OnRelease() = 0;
	virtual void Reset() = 0;
	virtual HRESULT RenderFrame(IDirect3DSurface9**, const SAMPLE_PRESENTATION&) = 0;
	virtual HRESULT RenderFrame(ID3D11Texture2D*, const SAMPLE_PRESENTATION&) = 0;
	virtual HRESULT RenderBlackFrame() = 0;
	virtual HRESULT RenderLastFrame() = 0;
	virtual HRESULT RenderLastFramePresentation(IDirect3DSurface9**) = 0;
	virtual BOOL GetDxva2Settings(DXVAHD_FILTER_RANGE_DATA_EX*, BOOL&) = 0;
	virtual HRESULT ResetDxva2Settings() = 0;
	virtual HRESULT SetFilter(const UINT, const INT) = 0;
	virtual IDirect3DDeviceManager9* GetDeviceManager9() = 0;
	virtual const BOOL IsInitialized() const = 0;
};

#endif //CDXRenderer