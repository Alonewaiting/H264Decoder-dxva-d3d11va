//----------------------------------------------------------------------------------------------
// DXDecoder.h
//----------------------------------------------------------------------------------------------
#ifndef DXDecoder_H
#define DXDecoder_H
#include "StdAfx.h"
class ID3D11Texture2D;
class DXDecoder{

public:

	virtual ~DXDecoder(){  }

	virtual HRESULT InitVideoDecoder(void*, const DXVA2_VideoDesc*, const SPS_DATA&) = 0;
	virtual void OnRelease() = 0;
	virtual void Reset() =0 ;
	virtual HRESULT DecodeFrame(CMFBuffer&, const PICTURE_INFO&, const LONGLONG&, const int) = 0;
	virtual BOOL CheckFrame(SAMPLE_PRESENTATION&) = 0;
	virtual HRESULT AddSliceShortInfo(const int, const DWORD, const BOOL) = 0;
	virtual void FreeSurfaceIndexRenderer(const DWORD) =0;

	// Inline
	virtual void ClearPresentation() = 0;
	virtual DWORD PictureToDisplayCount() const =0;
	virtual void SetCurrentNalu(const NAL_UNIT_TYPE eNalUnitType, const BYTE btNalRefIdc) = 0;
    //virtual IDirect3DSurface9** GetDirect3DSurface9() = 0;
    //virtual ID3D11Texture2D* GetD3D11Texture() = 0;
	virtual void* GetSurface(const DWORD index = 0) = 0;
	virtual const BOOL IsInitialized() const = 0;

};

#endif