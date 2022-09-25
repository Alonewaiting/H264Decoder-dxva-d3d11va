#include"StdAfx.h"
#include "CD3D11vaDecoder.h"

CD3D11vaDecoder::CD3D11vaDecoder()
{
}

CD3D11vaDecoder::~CD3D11vaDecoder()
{
}

HRESULT CD3D11vaDecoder::InitVideoDecoder(IDirect3DDeviceManager9*, const DXVA2_VideoDesc*, const SPS_DATA&)
{
    return E_NOTIMPL;
}

void CD3D11vaDecoder::OnRelease()
{
}

void CD3D11vaDecoder::Reset()
{
}

HRESULT CD3D11vaDecoder::DecodeFrame(CMFBuffer&, const PICTURE_INFO&, const LONGLONG&, const int)
{
    return E_NOTIMPL;
}

BOOL CD3D11vaDecoder::CheckFrame(SAMPLE_PRESENTATION&)
{
    return 0;
}

HRESULT CD3D11vaDecoder::AddSliceShortInfo(const int, const DWORD, const BOOL)
{
    return E_NOTIMPL;
}

void CD3D11vaDecoder::FreeSurfaceIndexRenderer(const DWORD)
{
}

void CD3D11vaDecoder::ClearPresentation()
{
}

DWORD CD3D11vaDecoder::PictureToDisplayCount() const
{
    return 0;
}

void CD3D11vaDecoder::SetCurrentNalu(const NAL_UNIT_TYPE eNalUnitType, const BYTE btNalRefIdc)
{
}

IDirect3DSurface9** CD3D11vaDecoder::GetDirect3DSurface9()
{
    return nullptr;
}

const BOOL CD3D11vaDecoder::IsInitialized() const
{
    return 0;
}
