#pragma once 
#include "CDXRenderer.h"
#include "wrl/client.h"
#include "D3D11RenderData.h"
class D3D11Renderer :public CDXRenderer {
public:
    D3D11Renderer();
    virtual ~D3D11Renderer();
    // Dxva2Renderer.cpp
    virtual HRESULT InitDXVA2(const HWND, const UINT, const UINT, const UINT, const UINT, DXVA2_VideoDesc&, const MFTIME) override;
    virtual void OnRelease() override;
    virtual void Reset() override;
    virtual HRESULT RenderFrame(IDirect3DSurface9**, const SAMPLE_PRESENTATION&) override;
    HRESULT RenderFrame(ID3D11Texture2D*, const SAMPLE_PRESENTATION&) override;
    virtual HRESULT RenderBlackFrame() override;
    virtual HRESULT RenderLastFrame() override;
    virtual HRESULT RenderLastFramePresentation(IDirect3DSurface9**) override;
    virtual BOOL GetDxva2Settings(DXVAHD_FILTER_RANGE_DATA_EX*, BOOL&) override;
    virtual HRESULT ResetDxva2Settings() override;
    virtual HRESULT SetFilter(const UINT, const INT) override;
    virtual IDirect3DDeviceManager9* GetDeviceManager9() override;
    virtual const BOOL IsInitialized() const override;


private:
    std::wstring                                        m_mainWndCaption;
    Size2d                                              m_winSize;
    HINSTANCE				                            m_hInstance;
    HWND					                            m_hWnd;
    D3D_DRIVER_TYPE			                            m_driverType;
    D3D_FEATURE_LEVEL		                            m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device           >     m_pd3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext    >     m_pImmediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain         >     m_pSwapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView >     m_pRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D        >     m_pDepthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView >     m_pDepthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_spRenderTexture;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
    Microsoft::WRL::ComPtr < ID3D11Buffer> m_pIndexBuffer;
    Microsoft::WRL::ComPtr < ID3D11InputLayout> m_pInputLayout;
    Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> m_pTexSRV[2];
    Microsoft::WRL::ComPtr < ID3D11SamplerState> m_pTexSampleState;
    SAMPLE_PRESENTATION m_LastPresentation;
    DWORD m_dwPicturePresent;
};