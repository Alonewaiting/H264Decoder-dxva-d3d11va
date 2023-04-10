/*!
 * \file D3D11RenderUtils.h
 *
 * \author dwy
 * \date 2023/1/2
 *
 * 
 */
#pragma once
#include <d3dcompiler.h>
class D3D11RenderUtils {
public:
    static HRESULT compileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob);


};