#include"StdAfx.h"
#include "CD3D11vaDecoder.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include "d3d11_4.h"
#if _DEBUG
#include <dxgidebug.h>
#endif
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#define  MAX_SURFACE_COUTN 64
/* define all the GUIDs used directly here,
 to avoid problems with inconsistent dxva2api.h versions in mingw-w64 and different MSVC version */
DEFINE_GUID(ff_DXVA2_ModeMPEG2_VLD, 0xee27417f, 0x5e28, 0x4e65, 0xbe, 0xea, 0x1d, 0x26, 0xb5, 0x08, 0xad, 0xc9);
DEFINE_GUID(ff_DXVA2_ModeMPEG2and1_VLD, 0x86695f12, 0x340e, 0x4f04, 0x9f, 0xd3, 0x92, 0x53, 0xdd, 0x32, 0x74, 0x60);
DEFINE_GUID(ff_DXVA2_ModeH264_E, 0x1b81be68, 0xa0c7, 0x11d3, 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5);
DEFINE_GUID(ff_DXVA2_ModeH264_F, 0x1b81be69, 0xa0c7, 0x11d3, 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5);
DEFINE_GUID(ff_DXVADDI_Intel_ModeH264_E, 0x604F8E68, 0x4951, 0x4C54, 0x88, 0xFE, 0xAB, 0xD2, 0x5C, 0x15, 0xB3, 0xD6);
DEFINE_GUID(ff_DXVA2_ModeVC1_D, 0x1b81beA3, 0xa0c7, 0x11d3, 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5);
DEFINE_GUID(ff_DXVA2_ModeVC1_D2010, 0x1b81beA4, 0xa0c7, 0x11d3, 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5);
DEFINE_GUID(ff_DXVA2_ModeHEVC_VLD_Main, 0x5b11d51b, 0x2f4c, 0x4452, 0xbc, 0xc3, 0x09, 0xf2, 0xa1, 0x16, 0x0c, 0xc0);
DEFINE_GUID(ff_DXVA2_ModeHEVC_VLD_Main10, 0x107af0e0, 0xef1a, 0x4d19, 0xab, 0xa8, 0x67, 0xa1, 0x63, 0x07, 0x3d, 0x13);
DEFINE_GUID(ff_DXVA2_ModeVP9_VLD_Profile0, 0x463707f8, 0xa1d0, 0x4585, 0x87, 0x6d, 0x83, 0xaa, 0x6d, 0x60, 0xb8, 0x9e);
DEFINE_GUID(ff_DXVA2_ModeVP9_VLD_10bit_Profile2, 0xa4c749ef, 0x6ecf, 0x48aa, 0x84, 0x48, 0x50, 0xa7, 0xa1, 0x16, 0x5f, 0xf7);
DEFINE_GUID(ff_DXVA2_ModeAV1_VLD_Profile0, 0xb8be4ccb, 0xcf53, 0x46ba, 0x8d, 0x59, 0xd6, 0xb8, 0xa6, 0xda, 0x5d, 0x2a);
DEFINE_GUID(ff_DXVA2_NoEncrypt, 0x1b81beD0, 0xa0c7, 0x11d3, 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5);
DEFINE_GUID(ff_GUID_NULL, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
DEFINE_GUID(ff_IID_IDirectXVideoDecoderService, 0xfc51a551, 0xd5e7, 0x11d9, 0xaf, 0x55, 0x00, 0x05, 0x4e, 0x43, 0xff, 0x02);

#define D3DALIGN(x, a) (((x)+(a)-1)&~((a)-1))


#define AV_CODEC_ID_H264 0
#include <fstream>
static std::string filePath = "D:/test/test.nv12";
static void saveFile(const char* data, const size_t& size) {
    std::fstream f;
    f.open(filePath,std::ios::out | std::ios::app);
    f.write(data,size);
    f.close();
}

CD3D11vaDecoder::CD3D11vaDecoder()
{
    
}

CD3D11vaDecoder::~CD3D11vaDecoder()
{
}

HRESULT CD3D11vaDecoder::InitVideoDecoder(IDirect3DDeviceManager9* pDirect3DDeviceManager9, const DXVA2_VideoDesc* pDxva2Desc, const SPS_DATA& sps)
{
    initDevice();
    initVideoDecoder(pDxva2Desc);
    InitDxva2Struct(sps);
    return 0;
}

void CD3D11vaDecoder::OnRelease()
{
   
    memset(m_BufferDesc, 0, 4 * sizeof(D3D11_VIDEO_DECODER_BUFFER_DESC));
    memset(&m_H264PictureParams, 0, sizeof(DXVA_PicParams_H264));
    memset(&m_H264QuantaMatrix, 0, sizeof(DXVA_Qmatrix_H264));
    memset(m_H264SliceShort, 0, MAX_SUB_SLICE * sizeof(DXVA_Slice_H264_Short));

    m_iPrevTopFieldOrderCount = 0;
    m_eNalUnitType = NAL_UNIT_UNSPEC_0;
    m_btNalRefIdc = 0x00;

    m_dwStatusReportFeedbackNumber = 1;

    m_dqPoc.clear();
    m_dqPicturePresentation.clear();

    ResetAllSurfaceIndex();
}

void CD3D11vaDecoder::Reset()
{
    m_iPrevTopFieldOrderCount = 0;
    m_eNalUnitType = NAL_UNIT_UNSPEC_0;
    m_btNalRefIdc = 0x00;
    m_dwStatusReportFeedbackNumber = 1;

    m_dqPoc.clear();
    m_dqPicturePresentation.clear();

    ResetAllSurfaceIndex();
}

HRESULT CD3D11vaDecoder::DecodeFrame(CMFBuffer& cMFNaluBuffer, const PICTURE_INFO& Picture, const LONGLONG& llTime, const int iSubSliceCount)
{
    HRESULT hr = S_OK;
    void* pBuffer = NULL;
    UINT uiSize = 0;
    static BOOL bFirst = TRUE;
    D3D11_VIDEO_DECODER_BUFFER_TYPE type;
    unsigned buffer_count = 0;
    assert(m_videoDecode != nullptr);

    DWORD dwCurPictureId = GetFreeSurfaceIndex();
    IF_FAILED_RETURN(dwCurPictureId == (DWORD)-1 ? E_FAIL : S_OK);

    try {
        do {

            hr = m_videoContext->DecoderBeginFrame(m_videoDecode.Get(),m_outputView[dwCurPictureId].Get(), 0,NULL);
            Sleep(1);
        } while (hr == E_PENDING);

        IF_FAILED_THROW(hr);
        // Picture
        InitPictureParams(dwCurPictureId, Picture);
        HandlePOC(dwCurPictureId, Picture, llTime);
        type = D3D11_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS;
        IF_FAILED_THROW(m_videoContext->GetDecoderBuffer(m_videoDecode.Get(),type,&uiSize,&pBuffer));
        assert(sizeof(DXVA_PicParams_H264) <= uiSize);
        memcpy(pBuffer, &m_H264PictureParams, sizeof(DXVA_PicParams_H264));
        IF_FAILED_THROW(m_videoContext->ReleaseDecoderBuffer(m_videoDecode.Get(), type));
        
        // QuantaMatrix
        type = D3D11_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX;
        IF_FAILED_THROW(m_videoContext->GetDecoderBuffer(m_videoDecode.Get(), type, &uiSize, &pBuffer));
        assert(sizeof(DXVA_Qmatrix_H264) <= uiSize);
        memcpy(pBuffer, &m_H264QuantaMatrix, sizeof(DXVA_Qmatrix_H264));
        IF_FAILED_THROW(m_videoContext->ReleaseDecoderBuffer(m_videoDecode.Get(), type));

        // BitStream
        type = D3D11_VIDEO_DECODER_BUFFER_BITSTREAM;
        IF_FAILED_THROW(m_videoContext->GetDecoderBuffer(m_videoDecode.Get(), type, &uiSize, &pBuffer));
        IF_FAILED_THROW(AddNalUnitBufferPadding(cMFNaluBuffer, uiSize));
        assert(cMFNaluBuffer.GetBufferSize() <= uiSize);
        memcpy(pBuffer, cMFNaluBuffer.GetStartBuffer(), cMFNaluBuffer.GetBufferSize());
        IF_FAILED_THROW(m_videoContext->ReleaseDecoderBuffer(m_videoDecode.Get(), type));

        // Slices
        type = D3D11_VIDEO_DECODER_BUFFER_SLICE_CONTROL;
        IF_FAILED_THROW(m_videoContext->GetDecoderBuffer(m_videoDecode.Get(), type, &uiSize, &pBuffer));
        assert(iSubSliceCount * sizeof(DXVA_Slice_H264_Short) <= uiSize);
        memcpy(pBuffer, m_H264SliceShort, iSubSliceCount * sizeof(DXVA_Slice_H264_Short));
        IF_FAILED_THROW(m_videoContext->ReleaseDecoderBuffer(m_videoDecode.Get(), type));

        // CompBuffers
        // Allready set
        m_BufferDesc[0].DataSize = sizeof(DXVA_PicParams_H264);
        m_BufferDesc[0].BufferType = D3D11_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS;
        m_BufferDesc[1].DataSize = sizeof(DXVA_Qmatrix_H264);
        m_BufferDesc[1].BufferType = D3D11_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX;

        m_BufferDesc[2].DataSize = cMFNaluBuffer.GetBufferSize();
        m_BufferDesc[2].BufferType = D3D11_VIDEO_DECODER_BUFFER_BITSTREAM;
        m_BufferDesc[2].NumMBsInBuffer = (Picture.sps.pic_width_in_mbs_minus1 + 1) * (Picture.sps.pic_height_in_map_units_minus1 + 1);
        m_BufferDesc[3].DataSize = iSubSliceCount * sizeof(DXVA_Slice_H264_Short);
        m_BufferDesc[3].NumMBsInBuffer = m_BufferDesc[2].NumMBsInBuffer;
        m_BufferDesc[3].BufferType = D3D11_VIDEO_DECODER_BUFFER_SLICE_CONTROL;

        IF_FAILED_THROW(m_videoContext->SubmitDecoderBuffers(m_videoDecode.Get(),4,m_BufferDesc));

        IF_FAILED_THROW(m_videoContext->DecoderEndFrame(m_videoDecode.Get()));
    }
    catch (HRESULT) {
        
    }
    //save NV12 Data
    if (true) {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        D3D11_TEXTURE2D_DESC desc2;
        m_texturePool->GetDesc(&desc2);

        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = m_videoDesc.SampleWidth;
        texDesc.Height = m_videoDesc.SampleHeight;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_NV12;
        texDesc.SampleDesc = desc2.SampleDesc;
        texDesc.ArraySize = 1;
        texDesc.Usage = D3D11_USAGE_STAGING;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        m_d3d11Device->CreateTexture2D(&texDesc, nullptr, texture.ReleaseAndGetAddressOf());
        m_deviceContext->CopySubresourceRegion(texture.Get(), 0, 0, 0, 0, m_texturePool.Get(), dwCurPictureId, NULL);
        D3D11_MAPPED_SUBRESOURCE map;
        m_deviceContext->Map(texture.Get(), 0, D3D11_MAP_READ, 0, &map);
        saveFile((char*)map.pData, map.DepthPitch);
        m_deviceContext->Unmap(texture.Get(), 0);
    } 
    return hr;

}
HRESULT CD3D11vaDecoder::AddNalUnitBufferPadding(CMFBuffer& cMFNaluBuffer, const UINT uiSize) {

    HRESULT hr;

    UINT uiPadding = MIN(128 - (cMFNaluBuffer.GetBufferSize() & 127), uiSize);

    IF_FAILED_RETURN(cMFNaluBuffer.Reserve(uiPadding));
    memset(cMFNaluBuffer.GetReadStartBuffer(), 0, uiPadding);
    IF_FAILED_RETURN(cMFNaluBuffer.SetEndPosition(uiPadding));

    return hr;
}
void CD3D11vaDecoder::InitPictureParams(const DWORD dwIndex, const PICTURE_INFO& Picture) {
    int iIndex = 0;

    if (m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR) {

        for (int i = 0; i < MAX_SURFACE_SIZE; i++) {
            g_Dxva2SurfaceIndexV2[i].bUsedByDecoder = FALSE;
            g_Dxva2SurfaceIndexV2[i].bNalRef = FALSE;
        }

        m_dqPoc.clear();
    }
    m_H264PictureParams.wFrameWidthInMbsMinus1 = (USHORT)Picture.sps.pic_width_in_mbs_minus1;
    m_H264PictureParams.wFrameHeightInMbsMinus1 = (USHORT)Picture.sps.pic_height_in_map_units_minus1;
    m_H264PictureParams.CurrPic.Index7Bits = dwIndex;
    m_H264PictureParams.CurrPic.AssociatedFlag = Picture.pps.bottom_field_pic_order_in_frame_present_flag;
    m_H264PictureParams.num_ref_frames = (UCHAR)Picture.sps.num_ref_frames;

    m_H264PictureParams.wBitFields = 0;
    m_H264PictureParams.field_pic_flag = 0;
    m_H264PictureParams.MbaffFrameFlag = 0;
    m_H264PictureParams.residual_colour_transform_flag = Picture.sps.separate_colour_plane_flag;
    m_H264PictureParams.sp_for_switch_flag = 0;
    m_H264PictureParams.chroma_format_idc = Picture.sps.chroma_format_idc;
    m_H264PictureParams.RefPicFlag = m_btNalRefIdc != 0;
    m_H264PictureParams.constrained_intra_pred_flag = Picture.pps.constrained_intra_pred_flag;
    m_H264PictureParams.weighted_pred_flag = Picture.pps.weighted_pred_flag;
    m_H264PictureParams.weighted_bipred_idc = Picture.pps.weighted_bipred_idc;
    m_H264PictureParams.MbsConsecutiveFlag = 1;
    m_H264PictureParams.frame_mbs_only_flag = Picture.sps.frame_mbs_only_flag;
    m_H264PictureParams.transform_8x8_mode_flag = Picture.pps.transform_8x8_mode_flag;
    m_H264PictureParams.MinLumaBipredSize8x8Flag = Picture.sps.level_idc >= 31;
    m_H264PictureParams.IntraPicFlag = (Picture.slice.slice_type == I_SLICE_TYPE || Picture.slice.slice_type == I_SLICE_TYPE2);

    m_H264PictureParams.bit_depth_luma_minus8 = (UCHAR)Picture.sps.bit_depth_luma_minus8;
    m_H264PictureParams.bit_depth_chroma_minus8 = (UCHAR)Picture.sps.bit_depth_chroma_minus8;
    m_H264PictureParams.Reserved16Bits = 3;
    m_H264PictureParams.StatusReportFeedbackNumber = m_dwStatusReportFeedbackNumber++;

    for (int i = 0; i < 16; i++) {

        m_H264PictureParams.RefFrameList[i].bPicEntry = 0xff;
        m_H264PictureParams.FieldOrderCntList[i][0] = m_H264PictureParams.FieldOrderCntList[i][1] = 0;
        m_H264PictureParams.FrameNumList[i] = 0;
    }

    m_H264PictureParams.CurrFieldOrderCnt[1] = m_H264PictureParams.CurrFieldOrderCnt[0] = Picture.slice.TopFieldOrderCnt;
    m_H264PictureParams.UsedForReferenceFlags = 0;

    for (auto it = m_dqPoc.begin(); it != m_dqPoc.end() && iIndex < 16; iIndex++, ++it) {

        m_H264PictureParams.FrameNumList[iIndex] = it->usFrameList;

        m_H264PictureParams.RefFrameList[iIndex].Index7Bits = it->bRefFrameList;
        m_H264PictureParams.RefFrameList[iIndex].AssociatedFlag = 0;
        m_H264PictureParams.FieldOrderCntList[iIndex][0] = m_H264PictureParams.FieldOrderCntList[iIndex][1] = it->TopFieldOrderCnt;
        m_H264PictureParams.UsedForReferenceFlags |= 1 << (2 * iIndex);
        m_H264PictureParams.UsedForReferenceFlags |= 1 << (2 * iIndex + 1);
    }

    m_H264PictureParams.pic_init_qs_minus26 = (CHAR)Picture.pps.pic_init_qs_minus26;
    m_H264PictureParams.chroma_qp_index_offset = (CHAR)Picture.pps.chroma_qp_index_offset[0];
    m_H264PictureParams.second_chroma_qp_index_offset = (CHAR)Picture.pps.chroma_qp_index_offset[1];
    m_H264PictureParams.ContinuationFlag = 1;
    m_H264PictureParams.pic_init_qp_minus26 = (CHAR)Picture.pps.pic_init_qp_minus26;
    m_H264PictureParams.num_ref_idx_l0_active_minus1 = (UCHAR)Picture.pps.num_ref_idx_l0_active_minus1;
    m_H264PictureParams.num_ref_idx_l1_active_minus1 = (UCHAR)Picture.pps.num_ref_idx_l1_active_minus1;
    m_H264PictureParams.Reserved8BitsA = 0;
    m_H264PictureParams.NonExistingFrameFlags = 0;
    m_H264PictureParams.frame_num = Picture.slice.frame_num;
    m_H264PictureParams.log2_max_frame_num_minus4 = (UCHAR)Picture.sps.log2_max_frame_num_minus4;
    m_H264PictureParams.pic_order_cnt_type = (UCHAR)Picture.sps.pic_order_cnt_type;
    m_H264PictureParams.log2_max_pic_order_cnt_lsb_minus4 = (UCHAR)Picture.sps.log2_max_pic_order_cnt_lsb_minus4;
    m_H264PictureParams.delta_pic_order_always_zero_flag = 0;
    m_H264PictureParams.direct_8x8_inference_flag = (UCHAR)Picture.sps.direct_8x8_inference_flag;
    m_H264PictureParams.entropy_coding_mode_flag = (UCHAR)Picture.pps.entropy_coding_mode_flag;
    m_H264PictureParams.pic_order_present_flag = (UCHAR)Picture.pps.bottom_field_pic_order_in_frame_present_flag;
    m_H264PictureParams.num_slice_groups_minus1 = (UCHAR)Picture.pps.num_slice_groups_minus1;
    m_H264PictureParams.slice_group_map_type = 0;
    m_H264PictureParams.deblocking_filter_control_present_flag = (UCHAR)Picture.pps.deblocking_filter_control_present_flag;
    m_H264PictureParams.redundant_pic_cnt_present_flag = (UCHAR)Picture.pps.redundant_pic_cnt_present_flag;
    m_H264PictureParams.Reserved8BitsB = 0;
    m_H264PictureParams.slice_group_change_rate_minus1 = (USHORT)Picture.pps.num_slice_groups_minus1;

    if (m_dwStatusReportFeedbackNumber == ULONG_MAX) {
        m_dwStatusReportFeedbackNumber = 1;
    }
  
}
void CD3D11vaDecoder::InitQuantaMatrixParams(const SPS_DATA& sps) {

    if (sps.bHasCustomScalingList == FALSE) {

        memset(&m_H264QuantaMatrix, 16, sizeof(DXVA_Qmatrix_H264));
    }
    else {

        memcpy(m_H264QuantaMatrix.bScalingLists4x4, sps.ScalingList4x4, sizeof(m_H264QuantaMatrix.bScalingLists4x4));
        memcpy(m_H264QuantaMatrix.bScalingLists8x8, sps.ScalingList8x8, sizeof(m_H264QuantaMatrix.bScalingLists8x8));
    }
}


BOOL CD3D11vaDecoder::CheckFrame(SAMPLE_PRESENTATION& SamplPresentation)
{
    BOOL bHasPicture = FALSE;

    for (deque<PICTURE_PRESENTATION>::const_iterator it = m_dqPicturePresentation.begin(); it != m_dqPicturePresentation.end(); ++it) {

        if (it->TopFieldOrderCnt == 0 || it->TopFieldOrderCnt == (m_iPrevTopFieldOrderCount + 2) || it->TopFieldOrderCnt == (m_iPrevTopFieldOrderCount + 1)) {

            SamplPresentation.dwDXVA2Index = it->dwDXVA2Index;
            m_iPrevTopFieldOrderCount = it->TopFieldOrderCnt;
            SamplPresentation.llTime = it->llTime;
            m_dqPicturePresentation.erase(it);
            bHasPicture = TRUE;
            break;
        }
    }

    // Use assert to check m_dqPicturePresentation size
    //assert(m_dqPicturePresentation.size() < NUM_DXVA2_SURFACE);

    // Check past frames, sometimes needed...
    if (bHasPicture)
        ErasePastFrames(SamplPresentation.llTime);

    return bHasPicture;
}
void CD3D11vaDecoder::InitDxva2Struct(const SPS_DATA& sps) {

   
    memset(m_BufferDesc, 0, 4 * sizeof(D3D11_VIDEO_DECODER_BUFFER_DESC));
    memset(&m_H264PictureParams, 0, sizeof(DXVA_PicParams_H264));
    InitQuantaMatrixParams(sps);
    memset(m_H264SliceShort, 0, MAX_SUB_SLICE * sizeof(DXVA_Slice_H264_Short));

    m_BufferDesc[0].BufferType = D3D11_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS;
    m_BufferDesc[0].DataSize = sizeof(DXVA_PicParams_H264);

    m_BufferDesc[1].BufferType = D3D11_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX;
    m_BufferDesc[1].DataSize = sizeof(DXVA_Qmatrix_H264);

    m_BufferDesc[2].BufferType = D3D11_VIDEO_DECODER_BUFFER_BITSTREAM;

    m_BufferDesc[3].BufferType = D3D11_VIDEO_DECODER_BUFFER_SLICE_CONTROL;

}
void CD3D11vaDecoder::ErasePastFrames(const LONGLONG llTime) {

    for (deque<PICTURE_PRESENTATION>::const_iterator it = m_dqPicturePresentation.begin(); it != m_dqPicturePresentation.end();) {
        if (it->llTime <= llTime)
            it = m_dqPicturePresentation.erase(it);
        else
            ++it;
    }
}
HRESULT CD3D11vaDecoder::AddSliceShortInfo(const int iSubSliceCount, const DWORD dwSize, const BOOL b4BytesStartCode)
{
    HRESULT hr;
    IF_FAILED_RETURN(iSubSliceCount > 0 && iSubSliceCount < MAX_SUB_SLICE ? S_OK : E_INVALIDARG);

    const int iIndex = iSubSliceCount - 1;

    m_H264SliceShort[iIndex].SliceBytesInBuffer = dwSize;
    m_H264SliceShort[iIndex].BSNALunitDataLocation = 0;
    m_H264SliceShort[iIndex].wBadSliceChopping = 0;

    if (b4BytesStartCode) {

        m_H264SliceShort[iIndex].SliceBytesInBuffer--;
        m_H264SliceShort[iIndex].BSNALunitDataLocation++;
    }

    for (int i = 1; i < iSubSliceCount; i++)
        m_H264SliceShort[iIndex].BSNALunitDataLocation += (m_H264SliceShort[i - 1].SliceBytesInBuffer + (b4BytesStartCode ? 1 : 0));

    return hr;
}

void CD3D11vaDecoder::FreeSurfaceIndexRenderer(const DWORD dwSurfaceIndex)
{
    assert(dwSurfaceIndex >= 0 && dwSurfaceIndex < MAX_SURFACE_SIZE);

    std::unique_lock<std::mutex> _(m_lock);

    g_Dxva2SurfaceIndexV2[dwSurfaceIndex].bUsedByRenderer = FALSE;
}

void CD3D11vaDecoder::HandlePOC(const DWORD dwIndex, const PICTURE_INFO& Picture, const LONGLONG& llTime) {

    if (m_btNalRefIdc && Picture.slice.PicMarking.adaptive_ref_pic_marking_mode_flag == FALSE && m_dqPoc.size() >= Picture.sps.num_ref_frames) {

        if (m_dqPoc.size() != 0) {

            UCHAR uc = m_dqPoc.back().bRefFrameList;
            g_Dxva2SurfaceIndexV2[uc].bUsedByDecoder = FALSE;
            g_Dxva2SurfaceIndexV2[uc].bNalRef = FALSE;
        }

        m_dqPoc.pop_back();
    }

    if (m_btNalRefIdc) {

        POC NewPoc;

        NewPoc.usFrameList = Picture.slice.frame_num;
        NewPoc.bRefFrameList = (UCHAR)dwIndex;
        NewPoc.TopFieldOrderCnt = Picture.slice.TopFieldOrderCnt;
        NewPoc.usSliceType = Picture.slice.slice_type;
        NewPoc.bLongRef = FALSE;

        m_dqPoc.push_front(NewPoc);

        for (int i = 0; i < ARRAYSIZE(g_Dxva2SurfaceIndexV2); i++) {

            if (g_Dxva2SurfaceIndexV2[i].bUsedByDecoder && g_Dxva2SurfaceIndexV2[i].bNalRef == FALSE)
                g_Dxva2SurfaceIndexV2[i].bUsedByDecoder = FALSE;
        }
    }

    g_Dxva2SurfaceIndexV2[dwIndex].bUsedByDecoder = TRUE;
    g_Dxva2SurfaceIndexV2[dwIndex].bNalRef = m_btNalRefIdc != 0x00;

    PICTURE_PRESENTATION pp;
    pp.TopFieldOrderCnt = Picture.slice.TopFieldOrderCnt;
    pp.dwDXVA2Index = dwIndex;
    pp.SliceType = (SLICE_TYPE)Picture.slice.slice_type;
    pp.llTime = llTime;

    if (Picture.slice.TopFieldOrderCnt != 0)
        m_dqPicturePresentation.push_front(pp);
    else
        m_dqPicturePresentation.push_back(pp);

    if (!m_btNalRefIdc)
        return;

    UINT uiCurIndex;
    MMCO_OP op;

    //todo 8.2.4 list reordering
    for (int iIndex = 0; iIndex < Picture.slice.PicMarking.iNumOPMode; iIndex++) {

        op = Picture.slice.PicMarking.mmcoOPMode[iIndex].mmcoOP;

        if (op == MMCO_OP_SHORT2UNUSED) {

            uiCurIndex = Picture.slice.PicMarking.mmcoOPMode[iIndex].difference_of_pic_nums_minus1 + 1;

            if (uiCurIndex >= m_dqPoc.size())
                uiCurIndex = (UINT)(m_dqPoc.size() - 1);

            if (uiCurIndex < m_dqPoc.size()) {

                auto it = m_dqPoc.begin() + uiCurIndex;

                g_Dxva2SurfaceIndexV2[it->bRefFrameList].bUsedByDecoder = FALSE;
                g_Dxva2SurfaceIndexV2[it->bRefFrameList].bNalRef = FALSE;

                m_dqPoc.erase(it);
            }
        }
        else if (op == MMCO_OP_SHORT2LONG) {

            for (auto it = m_dqPoc.begin(); it != m_dqPoc.end(); ++it) {

                if (it->TopFieldOrderCnt == Picture.slice.PicMarking.mmcoOPMode[iIndex].Long_term_frame_idx) {

                    it->bLongRef = TRUE;
                    break;
                }
            }
        }
    }
}

void CD3D11vaDecoder::ClearPresentation()
{
    m_dqPicturePresentation.clear(); 
    memset(g_Dxva2SurfaceIndexV2, 0, sizeof(g_Dxva2SurfaceIndexV2));
}

DWORD CD3D11vaDecoder::PictureToDisplayCount() const
{
    return (DWORD)m_dqPicturePresentation.size();
}

void CD3D11vaDecoder::SetCurrentNalu(const NAL_UNIT_TYPE eNalUnitType, const BYTE btNalRefIdc)
{
    m_eNalUnitType = eNalUnitType; m_btNalRefIdc = btNalRefIdc;
}
DWORD CD3D11vaDecoder::GetFreeSurfaceIndex() {

    DWORD dwSurfaceIndex = (DWORD)-1;

    std::unique_lock<std::mutex> _(m_lock);
    for (DWORD dw = 0; dw < NUM_DXVA2_SURFACE; dw++) {

        if (g_Dxva2SurfaceIndexV2[dw].bUsedByDecoder == FALSE && g_Dxva2SurfaceIndexV2[dw].bUsedByRenderer == FALSE) {

            //g_Dxva2SurfaceIndexV2[dw].bUsedByDecoder = TRUE;
            g_Dxva2SurfaceIndexV2[dw].bUsedByRenderer = TRUE;
            dwSurfaceIndex = dw;
            break;
        }
    }

    return dwSurfaceIndex;
}

void CD3D11vaDecoder::ResetAllSurfaceIndex() {

    std::unique_lock<std::mutex> _(m_lock);
    memset(g_Dxva2SurfaceIndexV2, 0, sizeof(g_Dxva2SurfaceIndexV2));
}

IDirect3DSurface9** CD3D11vaDecoder::GetDirect3DSurface9()
{
    return nullptr;
}

const BOOL CD3D11vaDecoder::IsInitialized() const
{
    return m_videoDecode != NULL;
}

HRESULT CD3D11vaDecoder::initDevice()
{
    if (m_d3d11Device) {
        return 0;
    }
    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    HRESULT re  = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factory.ReleaseAndGetAddressOf());
    if (FAILED(re)) {
        return re;
    }
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    for (int i = 0;i < 10; i++) {
       if (SUCCEEDED(factory->EnumAdapters(i, dxgiAdapter.ReleaseAndGetAddressOf()))) {
           break;
       }
    }
    if (!dxgiAdapter) {
        return re;
    }
    UINT creationFlags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#if _DEBUG
     creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    DXGI_ADAPTER_DESC desc;
    dxgiAdapter->GetDesc(&desc);

    re = D3D11CreateDevice(dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, creationFlags, NULL, 0,
        D3D11_SDK_VERSION,m_d3d11Device.ReleaseAndGetAddressOf(), NULL, NULL);
   if (FAILED(re)) {
       return re;
    }
   Microsoft::WRL::ComPtr<ID3D11Multithread> multithread;
    m_d3d11Device->QueryInterface(__uuidof(ID3D11Multithread),(void**)multithread.ReleaseAndGetAddressOf());
    if (multithread) {
        multithread->SetMultithreadProtected(true);
    }
    m_d3d11Device->GetImmediateContext(m_deviceContext.ReleaseAndGetAddressOf());
    m_d3d11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)m_videoDevice.ReleaseAndGetAddressOf());
    if (m_deviceContext) {
        m_deviceContext->QueryInterface(__uuidof(ID3D11VideoContext), (void**)m_videoContext.ReleaseAndGetAddressOf());
    }
    
    return re;
}

HRESULT CD3D11vaDecoder::initVideoDecoder(const DXVA2_VideoDesc* pDxva2Desc)
{
    if (!m_d3d11Device) {
        return E_NOTIMPL;
    }
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc ={};
    m_videoDesc = *pDxva2Desc;
    texDesc.Width = pDxva2Desc->SampleWidth;
    texDesc.Height = pDxva2Desc->SampleHeight;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_NV12;
    texDesc.SampleDesc.Count = 1;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = MAX_SURFACE_SIZE;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DECODER;
    texDesc.CPUAccessFlags = 0;
    m_d3d11Device->CreateTexture2D(&texDesc,nullptr,m_texturePool.ReleaseAndGetAddressOf());
    
    if (!m_texturePool) {
        return E_NOTIMPL;
    }
    m_guid = DXVA2_ModeH264_E;
    D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.DecodeProfile = m_guid;
    viewDesc.ViewDimension = D3D11_VDOV_DIMENSION_TEXTURE2D;
    m_outputView.resize(MAX_SURFACE_SIZE);
    for (int i = 0; i < MAX_SURFACE_SIZE; ++i) {
        viewDesc.Texture2D.ArraySlice = i;
        hr = m_videoDevice->CreateVideoDecoderOutputView((ID3D11Resource*)m_texturePool.Get(),&viewDesc,m_outputView[i].ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            return hr;
        }
    }

    D3D11_VIDEO_DECODER_DESC decoderDesc;
    ZeroMemory(&decoderDesc, sizeof(decoderDesc));
    decoderDesc.Guid = m_guid;
    decoderDesc.SampleWidth = pDxva2Desc->SampleWidth;
    decoderDesc.SampleHeight = pDxva2Desc->SampleHeight;
    decoderDesc.OutputFormat = DXGI_FORMAT_NV12;
    UINT cfg_count;

    hr = m_videoDevice->GetVideoDecoderConfigCount( &decoderDesc, &cfg_count);
    if (FAILED(hr)) {
        return hr;
    }
    /* List all configurations available for the decoder */
    D3D11_VIDEO_DECODER_CONFIG cfg_list[MAX_SURFACE_SIZE];
    for (unsigned i = 0; i < cfg_count; i++) {
        hr = m_videoDevice->GetVideoDecoderConfig(&decoderDesc, i, &cfg_list[i]);
        if (FAILED(hr)) {
            
            return  hr;
        }
    }
    int codec_id = 0;
    /* Select the best decoder configuration */
    int cfg_score = 0;
    D3D11_VIDEO_DECODER_CONFIG bestConfig;
    for (unsigned i = 0; i < cfg_count; i++) {
        const D3D11_VIDEO_DECODER_CONFIG* cfg = &cfg_list[i];

        /* */
        int score;
        if (cfg->ConfigBitstreamRaw == 1)
            score = 1;
        else if (codec_id == AV_CODEC_ID_H264 && cfg->ConfigBitstreamRaw == 2)
            score = 2;
        else
            continue;
        if (IsEqualGUID(cfg->guidConfigBitstreamEncryption, ff_DXVA2_NoEncrypt))
            score += 16;

        if (cfg_score < score) {
            bestConfig = *cfg;
            cfg_score = score;
        }
    }
    if (cfg_score <= 0) {   
        return E_NOTIMPL;
    }
    /* Create the decoder */
   
    hr = m_videoDevice->CreateVideoDecoder(&decoderDesc, &bestConfig, m_videoDecode.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
       
        return hr;
    }
    return hr;
}
