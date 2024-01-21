#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")


#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <stdio.h>
#include <iostream>
#include <math.h>  // sin, cos
#include "xdata.h" // 3d model

#include <SDL.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// constants are uniforms basically?
struct Constants
{
   glm::mat4 Transform;
   glm::mat4 Projection;
};

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "C:/repos/togl/togl/src/RendererD3D11.cpp"

struct float3 { float x, y, z; };

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
   swapChainDesc.Width              = 0; // use window wCreateDidth
   swapChainDesc.Height             = 0; // use window height
   swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
   swapChainDesc.Stereo             = FALSE;
   swapChainDesc.SampleDesc.Count   = 1;
   swapChainDesc.SampleDesc.Quality = 0;
   swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapChainDesc.BufferCount        = 2;
   swapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
   swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD; // prefer DXGI_SWAP_EFFECT_FLIP_DISCARD, see Minimal D3D11 pt2 
   swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
   swapChainDesc.Flags              = 0;
   
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

int main(int argc, char** args)
{
   SDL_Window* sdl_window = initSDL_Window();

   ID3D11Device1* device;
   ID3D11DeviceContext1* deviceContext;
   IDXGISwapChain1* swapChain;
   ID3D11Texture2D* frameBuffer;
   CreateDeviceAndSwapchain(&frameBuffer, &swapChain, &device, &deviceContext);

   ID3D11RenderTargetView* frameBufferView;
   device->CreateRenderTargetView(frameBuffer, nullptr, &frameBufferView);

   D3D11_TEXTURE2D_DESC depthBufferDesc;
   frameBuffer->GetDesc(&depthBufferDesc); // copy from framebuffer properties
   depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
   depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

   ID3D11Texture2D* depthBuffer;
   ID3D11DepthStencilView* depthBufferView;
   device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);
   device->CreateDepthStencilView(depthBuffer, nullptr, &depthBufferView);


   D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = // float3 position, float3 normal, float2 texcoord, float3 color
   {
       { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
       { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };
   ID3D11VertexShader* vertexShader;
   ID3D11PixelShader* pixelShader;
   ID3D11InputLayout* inputLayout;
   ParseAndCreateShaders(
      L"sprite.hlsl",
      &vertexShader, &pixelShader,
      &inputLayout, inputElementDesc, ARRAYSIZE(inputElementDesc),
      device);
   
   D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
   rasterizerDesc.FillMode = D3D11_FILL_SOLID;
   rasterizerDesc.CullMode = D3D11_CULL_BACK;
   ID3D11RasterizerState1* rasterizerState;
   device->CreateRasterizerState1(&rasterizerDesc, &rasterizerState);

   ID3D11SamplerState* samplerState = CreateSampler(device);

   D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
   depthStencilDesc.DepthEnable = TRUE;
   depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
   depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
   ID3D11DepthStencilState* depthStencilState;
   device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);



   // round constant buffer size to 16 byte boundary
   D3D11_BUFFER_DESC constantBufferDesc = {};
   constantBufferDesc.ByteWidth      = sizeof(Constants) + 0xf & 0xfffffff0;
   constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
   constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
   constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   ID3D11Buffer* constantBuffer;
   device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
   
   /// VERTEX BUFFER
   /// Index

   float Vdata[] = {   0.0,  0.0, 0.0, 0.0,
                       1.0,  0.0, 0.0, 1.0,
                       1.0,  1.0, 1.0, 1.0,
                       0.0,  1.0, 1.0, 0.0};
   D3D11_BUFFER_DESC vertexBufferDesc = {};
   vertexBufferDesc.ByteWidth = sizeof(Vdata);
   vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
   vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   D3D11_SUBRESOURCE_DATA vertexData = { Vdata };

   ID3D11Buffer* vertexBuffer;
   device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
 
   unsigned int iData[] = { 0, 2, 1, 0, 3, 2 };
   D3D11_BUFFER_DESC indexBufferDesc = {};
   indexBufferDesc.ByteWidth = sizeof(iData);
   indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
   indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   D3D11_SUBRESOURCE_DATA indexData = {iData};

   ID3D11Buffer* indexBuffer;
   device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

   /// Texture
   ID3D11Texture2D* texture = CreateTexture(device, "not used", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
   ID3D11ShaderResourceView* textureView;
   device->CreateShaderResourceView(texture, nullptr, &textureView);

   FLOAT backgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

   UINT stride = 4 * sizeof(float); // vertex size (11 floats: float3 position, float3 normal, float2 texcoord, float3 color)
   UINT offset = 0;

   D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(depthBufferDesc.Width), static_cast<float>(depthBufferDesc.Height), 0.0f, 1.0f };
   float w = viewport.Width / viewport.Height; float h = 1.0f; float n = 1.f; float f = 10.0f;


   deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   deviceContext->IASetInputLayout(inputLayout);
   deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
   deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

   deviceContext->VSSetShader(vertexShader, nullptr, 0);
   deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

   deviceContext->RSSetViewports(1, &viewport);
   deviceContext->RSSetState(rasterizerState);
   
   deviceContext->PSSetShader(pixelShader, nullptr, 0);
   deviceContext->PSSetShaderResources(0, 1, &textureView);
   deviceContext->PSSetSamplers(0, 1, &samplerState);

   deviceContext->OMSetRenderTargets(1, &frameBufferView, depthBufferView);
   //deviceContext->OMSetDepthStencilState(depthStencilState, 0);
   deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // use default blend mode (i.e. disable)
   
   const uint8_t* keys = SDL_GetKeyboardState(nullptr);
   float speed = 10;

   float3 modelRotation = { 0.0f, 0.0f, 0.0f };
   float3 modelScale = { 400.0f, 200.0f, 0.0f };
   float3 modelTranslation = { 100.0f, 100.0f, 0.0f };

   D3D11_MAPPED_SUBRESOURCE mappedSubresource;
   deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
   Constants* constants = reinterpret_cast<Constants*>(mappedSubresource.pData);
   constants->Projection = glm::orthoLH(0.0f, viewport.Width, 0.0f, viewport.Height, 0.1f, 10.0f);
   deviceContext->Unmap(constantBuffer, 0);

   while (true)
   {
      SDL_PumpEvents();
      if (keys[SDL_SCANCODE_ESCAPE]) break;
      if (keys[SDL_SCANCODE_UP]) modelTranslation.y    += speed;
      if (keys[SDL_SCANCODE_DOWN]) modelTranslation.y  -= speed;
      if (keys[SDL_SCANCODE_RIGHT]) modelTranslation.x += speed;
      if (keys[SDL_SCANCODE_LEFT]) modelTranslation.x  -= speed;

      deviceContext->ClearRenderTargetView(frameBufferView, backgroundColor);
      deviceContext->ClearDepthStencilView(depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

      Context myContext =
      {
         deviceContext, viewport, constantBuffer,
      };

      DrawQuad({ modelScale.x, modelScale.y }, { modelTranslation.x, modelTranslation.y }, 0, {}, myContext);


      swapChain->Present(1, 0);
   }
   return 0;
}