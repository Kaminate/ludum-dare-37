#pragma once
#include <d3d11.h>
#include "utility.h"

enum class Format
{
  r32g32b32float,
  r32g32float,

  r16uint,

  r8g8b8a8unorm,
  r8unorm,
};

struct LayoutCreator
{
  UINT alignedByteOffset = 0;
  std::vector< D3D11_INPUT_ELEMENT_DESC > layout;
  void AddLayout( const char* SemanticName, Format format );
};

struct Texture
{
  ID3D11Texture2D* texture;
  ID3D11ShaderResourceView* srv;
  uint32_t width;
  uint32_t height;
};

struct Shader
{
  ID3D11VertexShader* vertexShader;
  ID3D11PixelShader* pixelShader;
  ID3DBlob* vsBlob;
  ID3DBlob* psBlob;
};

struct Backbuffer
{
  ID3D11RenderTargetView* backbufferRTV;
  ID3D11DepthStencilView* backbufferDepthStencilView;
};

struct InputLayout
{
  ID3D11InputLayout* inputLayout;
};

struct VertexBuffer
{
  ID3D11Buffer* buffer;
  UINT stride;
};

struct IndexBuffer
{
  ID3D11Buffer* buffer;
  UINT indexCount;
  DXGI_FORMAT format;
};

struct ConstantBuffer
{
  ID3D11Buffer* buffer;
};

struct Blend
{
  ID3D11BlendState* state;
};

struct Depth
{
  ID3D11DepthStencilState* state;
};

struct Sampler
{
  ID3D11SamplerState* state;
};

struct Graphics
{
  Graphics( HWND windowHandle, UINT width, UINT height );
  ~Graphics();
  ID3D11Device* device;
  ID3D11DeviceContext* immediateContext;
  IDXGISwapChain* swapChain;

  // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476517(v=vs.85).aspx
  // A render-target view can be bound to the output-merger stage
  // by calling ID3D11DeviceContext::OMSetRenderTargets.
  ID3D11RenderTargetView* backbufferRTV;
  ID3D11Texture2D* backbufferDepthStencil;
  ID3D11DepthStencilView* backbufferDepthStencilView;

  void SetViewport( float width, float height );
  void SwapBuffers();
  Backbuffer GetBackbuffer();
  void SetRenderTarget( Backbuffer backbuffer );
  void Clear( Backbuffer backbuffer, Color4 color );

  Shader LoadShader( const char* path );
  void FreeShader( Shader shader );
  void SetShader( Shader shader );

  InputLayout CreateInputLayout( LayoutCreator layoutCreator, Shader shader );
  void SetInputLayout( InputLayout layout );
  void FreeInputLayout( InputLayout inputLayout );

  VertexBuffer CreateVertexBuffer( void* bufferData, UINT bufferByteCount, UINT stride );
  void SetVertexBuffer( VertexBuffer vertexBuffer );
  void FreeVertexBuffer( VertexBuffer vertexBuffer );

  IndexBuffer CreateIndexBuffer(
    void* bufferData,
    UINT bufferByteCount,
    Format format,
    UINT indexCount );
  void SetIndexBuffer( IndexBuffer indexBuffer );
  void FreeIndexBuffer( IndexBuffer indexBuffer );

  ConstantBuffer CreateConstantBuffer( UINT bufferSize );
  void SetConstantBufferData( ConstantBuffer constantBuffer, void* data );
  void SetConstantBuffer( ConstantBuffer constantBuffer, UINT slotIndex );
  void FreeConstantBuffer( ConstantBuffer constantBuffer );

  void Draw( IndexBuffer indexBuffer );

  Texture CreateTexture(
    void* bytes,
    int width,
    int height,
    Format format,
    int stride
  );
  void FreeTexture( Texture texture );
  void SetTexture( Texture texture, int index );

  Blend CreateBlend();
  void SetBlend( Blend blend );
  void FreeBlend( Blend blend );

  Depth CreateDepth();
  void SetDepth( Depth depth );
  void FreeDepth( Depth depth );

  Sampler CreateSampler();
  void SetSampler( Sampler sampler, int slot );
  void FreeSampler( Sampler sampler );
};
