#include "StdAfx.h"
#include "D3D11Renderer.h"
#include "D3D11RenderUtils.h"
#include "direct.h"
#include <array>
#include "texture2dSample_PS.h"
#include "texture2dSample_VS.h"
using namespace DirectX;
#define CHECK_COM_PTR(ptr) \
    if(!(ptr)){            \
        return S_FALSE;    \
    }
D3D11Renderer::D3D11Renderer()
{

}

D3D11Renderer::~D3D11Renderer()
{
    OnRelease();
}

HRESULT D3D11Renderer::InitDXVA2(const HWND hWnd, const UINT uiWidth, const UINT uiHeight, const UINT uiNumerator, const UINT uiDenominator, DXVA2_VideoDesc& Dxva2Desc, const MFTIME llVideoDuration)
{
    HRESULT hr = S_OK;
    m_hWnd = hWnd;
    m_winSize.m_width = uiWidth;
    m_winSize.m_height = uiHeight;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width = m_winSize.m_width;
    sd.BufferDesc.Height = m_winSize.m_height;
    sd.BufferDesc.RefreshRate.Numerator = uiNumerator;
    sd.BufferDesc.RefreshRate.Denominator = uiDenominator;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    //create device and swapchain
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
    {
        hr = D3D11CreateDeviceAndSwapChain(nullptr, driverTypes[driverTypeIndex], nullptr,
            createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &m_pSwapChain,
            &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
        if (SUCCEEDED(hr))
        {
            m_driverType = driverTypes[driverTypeIndex];
            break;
        }
    }
    if (FAILED(hr))
        return hr;

    //create render target view
    Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
    if (FAILED(hr))
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_pRenderTargetView.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    //create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    descDepth.Width = m_winSize.m_width;
    descDepth.Height = m_winSize.m_height;
    descDepth.ArraySize = 1;
    descDepth.MipLevels = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_pd3dDevice->CreateTexture2D(&descDepth, nullptr, m_pDepthStencilBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }
    //create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &descDSV, &m_pDepthStencilView);
    if (FAILED(hr))
        return hr;

    m_pImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), 0);

    //setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(m_winSize.m_width);
    vp.Height = static_cast<float>(m_winSize.m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    m_pImmediateContext->RSSetViewports(1, &vp);
    //create vertex and vertex shader
    VertexPos vertices[] =
    {
        {XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},

    };
    D3D11_BUFFER_DESC vertexDesc{ 0 };
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof(VertexPos) * ARRAYSIZE(vertices);

    D3D11_SUBRESOURCE_DATA resourceData;
    resourceData.pSysMem = vertices;
    auto result = m_pd3dDevice->CreateBuffer(&vertexDesc, &resourceData, m_pVertexBuffer.GetAddressOf());
    if (FAILED(result)) {
        OutputDebugStringA("Failed to Create vetex Buffer!");
        return result;
    }
    Microsoft::WRL::ComPtr < ID3D11VertexShader> textureVS;
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif
   

    LPCSTR profile = (m_pd3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
    Microsoft::WRL::ComPtr <ID3DBlob> vsBuffer;
    //result = D3D11RenderUtils::compileShader(L"D:/myCode/H264Dxva2Decoder/H264Dxva2Decoder/texture2dSample.hlsl", "VS_Main", "vs_4_0", vsBuffer.GetAddressOf());
    //if (FAILED(result))
    //{
    //    OutputDebugStringA("Failed compiling vertex shader ");
    //    return result;
    //}
    result = m_pd3dDevice->CreateVertexShader(g_VStexture2dSample, ARRAYSIZE(g_VStexture2dSample), 0, textureVS.GetAddressOf());

    //create fragment shader
    Microsoft::WRL::ComPtr<ID3DBlob> fsBuffer;
    Microsoft::WRL::ComPtr <ID3D11PixelShader> texturePS;
    /*result = D3D11RenderUtils::compileShader(L"D:/myCode/H264Dxva2Decoder/H264Dxva2Decoder/texture2dSample.hlsl", "PS_Main", "ps_4_0", &fsBuffer);
    if (FAILED(result))
    {
        OutputDebugStringA("Failed compiling fragment shader ");
        return result;
    }*/
    result = m_pd3dDevice->CreatePixelShader(g_PStexture2dSample, ARRAYSIZE(g_PStexture2dSample), 0, texturePS.GetAddressOf());
    //input
    D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);
    result = m_pd3dDevice->CreateInputLayout(solidColorLayout,
        totalLayoutElements, g_VStexture2dSample,
        ARRAYSIZE(g_VStexture2dSample), m_pInputLayout.GetAddressOf());

    unsigned int stride = sizeof(VertexPos);
    unsigned int offset = 0;


    m_pImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pImmediateContext->VSSetShader(textureVS.Get(), 0, 0);
    m_pImmediateContext->PSSetShader(texturePS.Get(), 0, 0);

    bool forceSRGB = true;
    bool enableMips = true;

    // Éú³Émipmap
    //if (enableMips)
    //    m_pImmediateContext->GenerateMips(m_pTexSRV.Get());
    //color map sample 
    //color map sample 
    D3D11_SAMPLER_DESC colorMapDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());;
    result = m_pd3dDevice->CreateSamplerState(&colorMapDesc, m_pTexSampleState.GetAddressOf());
    if (FAILED(result)) {
        OutputDebugStringA("Failed CreateSamplerState ");
     
    }
    DXVA2_Frequency Dxva2Freq;
    Dxva2Freq.Numerator = uiNumerator;
    Dxva2Freq.Denominator = uiDenominator;

    Dxva2Desc.SampleWidth = uiWidth;
    Dxva2Desc.SampleHeight = uiHeight;

    Dxva2Desc.SampleFormat.SampleFormat = MFVideoInterlace_Progressive;

    Dxva2Desc.Format = static_cast<D3DFORMAT>(D3DFMT_NV12);
    Dxva2Desc.InputSampleFreq = Dxva2Freq;
    Dxva2Desc.OutputFrameFreq = Dxva2Freq;

    return result;
}

void D3D11Renderer::OnRelease()
{
   memset(&m_LastPresentation ,0 , sizeof(SAMPLE_PRESENTATION));
}

void D3D11Renderer::Reset()
{
    m_dwPicturePresent = 0;
    memset(&m_LastPresentation, 0, sizeof(m_LastPresentation));
}


HRESULT D3D11Renderer::RenderFrame(void* itexture, const SAMPLE_PRESENTATION& info)
{
    ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(itexture);
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    D3D11_TEXTURE2D_DESC sourceDesc;
    texture->GetDevice(device.ReleaseAndGetAddressOf());
    texture->GetDesc(&sourceDesc);
    //copy
    if (!m_spRenderTexture) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = sourceDesc.Width;
        desc.Height = sourceDesc.Height;
        desc.Format = sourceDesc.Format;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        desc.SampleDesc = sourceDesc.SampleDesc;
        m_pd3dDevice->CreateTexture2D(&desc, NULL, m_spRenderTexture.ReleaseAndGetAddressOf());
    }
    CHECK_COM_PTR(m_spRenderTexture);
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(context.ReleaseAndGetAddressOf());
    CHECK_COM_PTR(context);
    //share to decode device;
    Microsoft::WRL::ComPtr<IDXGIResource> shareResource;
    m_spRenderTexture->QueryInterface(__uuidof(IDXGIResource),(void**)shareResource.ReleaseAndGetAddressOf());
    CHECK_COM_PTR(shareResource);
    HANDLE shareHandle = nullptr;
    shareResource->GetSharedHandle(&shareHandle);
    Microsoft::WRL::ComPtr<IDXGIResource> targetResource;
    device->OpenSharedResource(shareHandle,__uuidof(ID3D11Texture2D), (void**)targetResource.ReleaseAndGetAddressOf());
    CHECK_COM_PTR(targetResource);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> tempTexture;
    targetResource->QueryInterface(__uuidof(ID3D11Texture2D),(void**)tempTexture.ReleaseAndGetAddressOf());
    CHECK_COM_PTR(tempTexture);
    context->CopySubresourceRegion(tempTexture.Get(),0,0,0,0,texture,info.dwDXVA2Index,nullptr);
    
    //render texture
    //create share resource view  SRV;
    if (m_pTexSRV[0] == nullptr || m_pTexSRV[1] == nullptr) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = sourceDesc.Format == DXGI_FORMAT_NV12 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R16_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        m_pd3dDevice->CreateShaderResourceView(m_spRenderTexture.Get(),&srvDesc,m_pTexSRV[0].ReleaseAndGetAddressOf());
        srvDesc.Format = sourceDesc.Format == DXGI_FORMAT_NV12 ? DXGI_FORMAT_R8G8_UNORM : DXGI_FORMAT_R16G16_UNORM;
        m_pd3dDevice->CreateShaderResourceView(m_spRenderTexture.Get(), &srvDesc, m_pTexSRV[1].ReleaseAndGetAddressOf());
    }
    CHECK_COM_PTR(m_pTexSRV[0]);
    CHECK_COM_PTR(m_pTexSRV[1]);
    std::array<ID3D11ShaderResourceView*, 2> const textureViews = {
        m_pTexSRV[0].Get(),
        m_pTexSRV[1].Get()
    };
    
    
    float clearColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
    m_pImmediateContext->IASetInputLayout(m_pInputLayout.Get());
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor);
    m_pImmediateContext->PSSetShaderResources(0,textureViews.size(),textureViews.data());
    m_pImmediateContext->PSSetSamplers(0, 1, m_pTexSampleState.GetAddressOf());
    m_pImmediateContext->Draw(6, 0);
    m_pSwapChain->Present(0, 0);
    return S_OK;
}

HRESULT D3D11Renderer::RenderBlackFrame()
{
    return {};
}

HRESULT D3D11Renderer::RenderLastFrame()
{
    HRESULT hr;

    IF_FAILED_RETURN(m_pSwapChain ? S_OK : E_UNEXPECTED);
    IF_FAILED_RETURN(m_pSwapChain->Present(NULL, NULL));
    return hr;
}

HRESULT D3D11Renderer::RenderLastFramePresentation(void*)
{
    return {};
}

BOOL D3D11Renderer::GetDxva2Settings(DXVAHD_FILTER_RANGE_DATA_EX*, BOOL&)
{
    return {};
}

HRESULT D3D11Renderer::ResetDxva2Settings()
{
    return {};
}

HRESULT D3D11Renderer::SetFilter(const UINT, const INT)
{
    return {};
}

void* D3D11Renderer::GetDevice()
{
    return {};
}

const BOOL D3D11Renderer::IsInitialized() const
{
    return true;
}

