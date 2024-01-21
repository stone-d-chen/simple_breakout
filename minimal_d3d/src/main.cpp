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


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "C:/repos/togl/togl/src/RendererD3D11.cpp"

struct float3 { float x, y, z; };


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
   
   /// VERTEX AND INDEX BUFFER LOAD
   float quadVertices[] = {   0.0,  0.0, 0.0, 0.0,
                       1.0,  0.0, 0.0, 1.0,
                       1.0,  1.0, 1.0, 1.0,
                       0.0,  1.0, 1.0, 0.0};
   D3D11_BUFFER_DESC vertexBufferDesc = {};
   vertexBufferDesc.ByteWidth = sizeof(quadVertices);
   vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
   vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   D3D11_SUBRESOURCE_DATA vertexData = { quadVertices };
   ID3D11Buffer* vertexBuffer;
   device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
 
   unsigned int indices[] = { 0, 2, 1, 0, 3, 2 };
   D3D11_BUFFER_DESC indexBufferDesc = {};
   indexBufferDesc.ByteWidth = sizeof(indices);
   indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
   indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   D3D11_SUBRESOURCE_DATA indexData = { indices };
   ID3D11Buffer* indexBuffer;
   device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

   /// Texture
   ID3D11Texture2D* texture = CreateTexture(device, "not used", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
   ID3D11ShaderResourceView* textureView;
   device->CreateShaderResourceView(texture, nullptr, &textureView);


   UINT stride = 4 * sizeof(float); // vertex size (11 floats: float3 position, float3 normal, float2 texcoord, float3 color)
   UINT offset = 0;
   deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   deviceContext->IASetInputLayout(inputLayout);
   deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
   deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

   deviceContext->VSSetShader(vertexShader, nullptr, 0);
   deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

   D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(depthBufferDesc.Width), static_cast<float>(depthBufferDesc.Height), 0.0f, 1.0f };
   float w = viewport.Width / viewport.Height; float h = 1.0f; float n = 1.f; float f = 10.0f;
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

   FLOAT backgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
   Context myContext =
   {
      deviceContext, viewport, constantBuffer,
   };

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


      DrawQuad(
         { modelScale.x, modelScale.y }, 
         { modelTranslation.x, modelTranslation.y }, 
         0, {}, myContext);


      swapChain->Present(1, 0);
   }
   return 0;
}