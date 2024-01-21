#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

struct Context
{
    ID3D11DeviceContext1* deviceContext;
    D3D11_VIEWPORT viewport;
    ID3D11Buffer* constantBuffer;
};

// constants are uniforms basically? // also this is related to the shader...
struct Constants
{
    glm::mat4 Transform;
    glm::mat4 Projection;
};


SDL_Window* initSDL_Window()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
    SDL_Window* sdl_window = SDL_CreateWindow("SDL2 with D3D11",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN);

    return sdl_window;
}

void init_D3D11(ID3D11Device1** device, ID3D11DeviceContext** deviceContext)
{
    // device is like the raw adapter for create buffers and such
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    ID3D11Device* baseDevice;

    // context is the command portion of it
    ID3D11DeviceContext* baseDeviceContext;
    D3D11CreateDevice(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);

    // I want the COM device1 and com devicecontext1
    // ID3D11Device1* device;
    // ID3D11DeviceContext1* deviceContext;

    baseDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(device));
    baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(deviceContext));
}

void CreateDeviceAndSwapchain(
    ID3D11Texture2D** frameBuffer,
    IDXGISwapChain1** swapChain,
    ID3D11Device1** device,
    ID3D11DeviceContext1** deviceContext)
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    ID3D11Device* baseDevice;

    ID3D11DeviceContext* baseDeviceContext;
    D3D11CreateDevice(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, &baseDevice,
        nullptr, &baseDeviceContext);

    baseDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(device));
    baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1),
        reinterpret_cast<void**>(deviceContext));

    IDXGIDevice1* dxgiDevice;
    (*device)->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgiDevice));
    IDXGIAdapter* dxgiAdapter;
    dxgiDevice->GetAdapter(&dxgiAdapter);

    IDXGIFactory2* dxgiFactory;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width = 0; // use window wCreateDidth
    swapChainDesc.Height = 0; // use window height
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // prefer DXGI_SWAP_EFFECT_FLIP_DISCARD, see Minimal D3D11 pt2 
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    HWND window = GetActiveWindow(); // maybe move this directly to whwere I need
    dxgiFactory->CreateSwapChainForHwnd(*device, window, &swapChainDesc, nullptr, nullptr, swapChain);

    (*swapChain)->GetBuffer(0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(frameBuffer));
}

void ParseAndCreateShaders(
    const WCHAR* name,
    ID3D11VertexShader** vertexShader,
    ID3D11PixelShader** pixelShader,
    ID3D11InputLayout** inputLayout,
    D3D11_INPUT_ELEMENT_DESC* inputElementDesc,
    UINT NumElements,
    ID3D11Device1* device)
{
    ID3DBlob* vsBlob;
    auto Result = D3DCompileFromFile(name,
        nullptr, nullptr,
        "vs_main", "vs_5_0",
        0, 0, &vsBlob, nullptr);
    device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        vertexShader);

    ID3DBlob* psBlob;
    D3DCompileFromFile(name, nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, nullptr);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShader);

    device->CreateInputLayout(
        inputElementDesc,
        NumElements,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        inputLayout);

}

ID3D11Texture2D* CreateTexture(ID3D11Device1* device, const char* filename, DXGI_FORMAT format)
{
    filename = "C:/repos/togl/togl/res/textures/paddle.png";
    int width, height, nrChannels;
    unsigned char* image = stbi_load(filename, &width, &height, &nrChannels, 0);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;  // in xdata.h
    textureDesc.Height = height; // in xdata.h
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = image;
    textureData.SysMemPitch = width * sizeof(UINT); // 4 bytes per pixel

    ID3D11Texture2D* texture;
    device->CreateTexture2D(&textureDesc, &textureData, &texture);
    return texture;
}


ID3D11SamplerState* CreateSampler(ID3D11Device1* device)
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    ID3D11SamplerState* samplerState;
    device->CreateSamplerState(&samplerDesc, &samplerState);
    return samplerState;
}


void DrawQuad(const glm::vec2& pixelDimensions, const glm::vec2& pixelPosition,
	float rotateRadians, const glm::vec4 Color, Context context)
{
    ID3D11DeviceContext* deviceContext = context.deviceContext;
    D3D11_VIEWPORT viewport = context.viewport;
    ID3D11Buffer* constantBuffer = context.constantBuffer;

    glm::mat4 model(1.0f);
    auto scale = glm::scale(model, glm::vec3(pixelDimensions, 0));
    auto translate = glm::translate(model, glm::vec3(pixelPosition, 0));

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    Constants* constants = reinterpret_cast<Constants*>(mappedSubresource.pData);
    constants->Transform = translate * scale * glm::mat4(1.0f);
    constants->Projection = glm::orthoLH(0.0f, viewport.Width, 0.0f, viewport.Height, 0.1f, 10.0f);
    deviceContext->Unmap(constantBuffer, 0);

    deviceContext->DrawIndexed(6, 0, 0);
}