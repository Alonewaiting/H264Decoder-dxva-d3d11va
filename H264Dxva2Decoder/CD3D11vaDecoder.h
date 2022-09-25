#pragma once
#include "DXDecoder.h"

class CD3D11vaDecoder : public DXDecoder {


public:
	CD3D11vaDecoder();
	virtual ~CD3D11vaDecoder();

	virtual HRESULT InitVideoDecoder(IDirect3DDeviceManager9*, const DXVA2_VideoDesc*, const SPS_DATA&) override;
	virtual void OnRelease() override;
	virtual void Reset() override;
	virtual HRESULT DecodeFrame(CMFBuffer&, const PICTURE_INFO&, const LONGLONG&, const int) override;
	virtual BOOL CheckFrame(SAMPLE_PRESENTATION&) override;
	virtual HRESULT AddSliceShortInfo(const int, const DWORD, const BOOL) override;
	virtual void FreeSurfaceIndexRenderer(const DWORD) override;

	// Inline
	virtual void ClearPresentation() override;
	virtual DWORD PictureToDisplayCount() const override;
	virtual void SetCurrentNalu(const NAL_UNIT_TYPE eNalUnitType, const BYTE btNalRefIdc) override;
	virtual IDirect3DSurface9** GetDirect3DSurface9() override;
	virtual const BOOL IsInitialized() const override;




};