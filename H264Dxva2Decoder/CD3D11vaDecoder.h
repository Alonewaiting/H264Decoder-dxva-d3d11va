#pragma once
#include "DXDecoder.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <map>
#include <mutex>
#define MAX_SURFACE_SIZE 64
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

private:
	HRESULT initDevice();
	HRESULT initVideoDecoder(const DXVA2_VideoDesc* pDxva2Desc);
	void InitPictureParams(const DWORD dwIndex, const PICTURE_INFO& Picture);
	void InitQuantaMatrixParams(const SPS_DATA& sps);
	void ErasePastFrames(const LONGLONG llTime);
	void HandlePOC(const DWORD dwIndex, const PICTURE_INFO& Picture, const LONGLONG& llTime);
	DWORD GetFreeSurfaceIndex();
	void ResetAllSurfaceIndex();
	void InitDxva2Struct(const SPS_DATA& sps);
	HRESULT AddNalUnitBufferPadding(CMFBuffer& cMFNaluBuffer, const UINT uiSize);
private:
	Microsoft::WRL::ComPtr <ID3D11Device> m_d3d11Device;
	Microsoft::WRL::ComPtr <ID3D11DeviceContext> m_deviceContext;
	Microsoft::WRL::ComPtr <ID3D11VideoDevice>  m_videoDevice;
	Microsoft::WRL::ComPtr <ID3D11VideoContext> m_videoContext;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texturePool;
	Microsoft::WRL::ComPtr < ID3D11VideoDecoder> m_videoDecode;
	std::vector<Microsoft::WRL::ComPtr<ID3D11VideoDecoderOutputView>> m_outputView;
	std::map<int, bool> m_textureState;
	GUID m_guid;
	NAL_UNIT_TYPE m_eNalUnitType;
    struct DXVA2_SURFACE_INDEX {

        BOOL bUsedByDecoder;
        BOOL bUsedByRenderer;
        BOOL bNalRef;
    };
    struct POC {

        UINT TopFieldOrderCnt;
        USHORT usFrameList;
        UCHAR bRefFrameList;
        USHORT usSliceType;
        BOOL bLongRef;
    };

    struct PICTURE_PRESENTATION {

        INT TopFieldOrderCnt;
        DWORD dwDXVA2Index;
        SLICE_TYPE SliceType;
        LONGLONG llTime;
    };

    deque<POC> m_dqPoc;
    deque<PICTURE_PRESENTATION> m_dqPicturePresentation;
	BYTE m_btNalRefIdc;
	INT m_iPrevTopFieldOrderCount;
	DXVA2_VideoDesc m_videoDesc;

    DXVA2_SURFACE_INDEX g_Dxva2SurfaceIndexV2[MAX_SURFACE_SIZE];
	D3D11_VIDEO_DECODER_BUFFER_DESC m_BufferDesc[4];
    DXVA_PicParams_H264 m_H264PictureParams;
    DXVA_Qmatrix_H264 m_H264QuantaMatrix;
    DXVA_Slice_H264_Short m_H264SliceShort[MAX_SUB_SLICE];
    DWORD m_dwStatusReportFeedbackNumber;
	std::mutex m_lock;
};