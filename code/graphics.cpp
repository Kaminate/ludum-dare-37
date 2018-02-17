#include "graphics.h"
#include <D3DCompiler.h>

static DXGI_FORMAT ToDXGI_Format( Format format )
{
  switch( format )
  {
    case Format::r32g32b32float: return DXGI_FORMAT_R32G32B32_FLOAT;
    case Format::r32g32float:return DXGI_FORMAT_R32G32_FLOAT;
    case Format::r16uint:return DXGI_FORMAT_R16_UINT;
    case Format::r8g8b8a8unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Format::r8unorm: return DXGI_FORMAT_R8_UNORM;
  }
  InvalidCodePath;
  return DXGI_FORMAT_UNKNOWN;
}

void LayoutCreator::AddLayout( const char* SemanticName, Format format )
{
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476180(v=vs.85).aspx
  D3D11_INPUT_ELEMENT_DESC desc = {};
  // SemanticName: The HLSL semantic associated with this element in a shader
  //   input-signature.
  desc.SemanticName = SemanticName;
  // Semantic Index: semantic index is only needed in a case where there is more than
  //   one element with the same semantic.
  switch( format )
  {
    case Format::r32g32b32float: desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
    case Format::r32g32float: desc.Format = DXGI_FORMAT_R32G32_FLOAT; break;
      InvalidDefaultCase;
  }
  // InputSlot: input-assembler (see input slot). Valid values are between 0 and 15
  // AlignedByteOffset: Optional. Offset (in bytes) between each element.
  //   Use D3D11_APPEND_ALIGNED_ELEMENT for convenience to define the current
  //   element directly after the previous one, including any packing if necessary.
  // InputSlotClass: either D3D11_INPUT_PER_VERTEX_DATA or D3D11_INPUT_PER_INSTANCE_DATA
  desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  desc.AlignedByteOffset = alignedByteOffset;
  switch( format )
  {
    case Format::r32g32b32float: alignedByteOffset += 12; break;
    case Format::r32g32float: alignedByteOffset += 8; break;
      InvalidDefaultCase;
  }
  layout.push_back( desc );
}

Graphics::Graphics(
  HWND windowHandle,
  UINT width,
  UINT height )
{
  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof( sd ) );
  sd.BufferCount = 1;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = windowHandle;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE; // <-- ?

  HRESULT hr = D3D11CreateDeviceAndSwapChain(
    NULL,
    driverType,
    NULL,
    createDeviceFlags,
    &featureLevel,
    1,
    D3D11_SDK_VERSION,
    &sd,
    &swapChain,
    &device,
    nullptr,
    &immediateContext );
  if( FAILED( hr ) )
    HandleErrorGracefully();

  ID3D11Texture2D* backbuffer;
  hr = swapChain->GetBuffer(
    0, __uuidof( ID3D11Texture2D ), ( void** )&backbuffer );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  hr = device->CreateRenderTargetView( backbuffer, nullptr, &backbufferRTV );
  backbuffer->Release();

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476253(v=vs.85).aspx
  // The maximum number of mipmap levels in the texture.
  // See the remarks in D3D11_TEX1D_SRV.
  // Use 1 for a multisampled texture; or 0 to generate a full set of subtextures.
  //
  // Question: What?
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  desc.SampleDesc.Count = 1;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  hr = device->CreateTexture2D( &desc, nullptr, &backbufferDepthStencil );
  if( FAILED( hr ) )
    HandleErrorGracefully();

  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
  descDSV.Format = desc.Format;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  hr = device->CreateDepthStencilView(
    backbufferDepthStencil, &descDSV, &backbufferDepthStencilView );
  if( FAILED( hr ) )
    HandleErrorGracefully();
}

Graphics::~Graphics()
{
  device->Release();
  immediateContext->Release();
  swapChain->Release();
  backbufferRTV->Release();
  backbufferDepthStencil->Release();
  backbufferDepthStencilView->Release();
}

void Graphics::SetViewport( float width, float height )
{
  D3D11_VIEWPORT viewport = {};
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  viewport.Width = width;
  viewport.Height = height;
  immediateContext->RSSetViewports( 1, &viewport );
}

void Graphics::SwapBuffers()
{
  UINT syncinterval = 0;
  UINT flags = 0;
  swapChain->Present( syncinterval, flags );
}

Backbuffer Graphics::GetBackbuffer()
{
  Backbuffer result;
  result.backbufferDepthStencilView = backbufferDepthStencilView;
  result.backbufferRTV = backbufferRTV;
  return result;
}

void Graphics::SetRenderTarget( Backbuffer backbuffer )
{
  ID3D11RenderTargetView* views[1] = { backbuffer.backbufferRTV };
  immediateContext->OMSetRenderTargets(
    1,
    views,
    backbuffer.backbufferDepthStencilView );
}

void Graphics::Clear( Backbuffer backbuffer, Color4 color )
{
  immediateContext->ClearRenderTargetView(
    backbuffer.backbufferRTV,
    &color.r );
  immediateContext->ClearDepthStencilView(
    backbuffer.backbufferDepthStencilView,
    D3D11_CLEAR_DEPTH,
    1.0f,
    0 );
}

ID3DBlob* CompileShader(
  const char* entryPoint,
  const char* target,
  const char* path )
{
  TemporaryMemory memory( path );
  ID3DBlob* blob;
  ID3DBlob* errors;

  UINT flags1 = 0;
#ifdef _DEBUG
  flags1 |= D3DCOMPILE_DEBUG;
#endif
  UINT flags2 = 0;
  HRESULT hr = D3DCompile(
    memory.mBytes,
    memory.mByteCount,
    nullptr,
    nullptr,
    nullptr,
    entryPoint,
    target,
    flags1,
    flags2,
    &blob,
    &errors );
  if( FAILED( hr ) )
  {
    const char* errorsReadable = ( const char* )errors->GetBufferPointer();
    HandleErrorGracefully( va( "%s\n%s",
      path,
      errorsReadable ) );
  }
  return blob;
}

Shader Graphics::LoadShader( const char* path )
{
  Shader shader;

  shader.vsBlob = CompileShader(
    "vsmain",
    "vs_5_0",
    path );
  shader.psBlob = CompileShader(
    "psmain",
    "ps_5_0",
    path );

  HRESULT hr = device->CreateVertexShader(
    shader.vsBlob->GetBufferPointer(),
    shader.vsBlob->GetBufferSize(),
    nullptr,
    &shader.vertexShader );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  hr = device->CreatePixelShader(
    shader.psBlob->GetBufferPointer(),
    shader.psBlob->GetBufferSize(),
    nullptr,
    &shader.pixelShader );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  return shader;
}

void Graphics::FreeShader( Shader shader )
{
  shader.pixelShader->Release();
  shader.vertexShader->Release();
  shader.vsBlob->Release();
  shader.psBlob->Release();
}

void Graphics::SetShader( Shader shader )
{
  immediateContext->VSSetShader( shader.vertexShader, nullptr, 0 );
  immediateContext->PSSetShader( shader.pixelShader, nullptr, 0 );
}

InputLayout Graphics::CreateInputLayout( LayoutCreator layoutCreator, Shader shader )
{
  InputLayout result;
  Assert( !layoutCreator.layout.empty() );
  HRESULT hr = device->CreateInputLayout(
    layoutCreator.layout.data(),
    ( UINT )layoutCreator.layout.size(),
    shader.vsBlob->GetBufferPointer(),
    shader.vsBlob->GetBufferSize(),
    &result.inputLayout );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  return result;
}

void Graphics::SetInputLayout( InputLayout layout )
{
  immediateContext->IASetInputLayout( layout.inputLayout );
}

void Graphics::FreeInputLayout( InputLayout inputLayout )
{
  inputLayout.inputLayout->Release();
}

VertexBuffer Graphics::CreateVertexBuffer(
  void* bufferData,
  UINT bufferByteCount,
  UINT stride )
{
  VertexBuffer result;
  result.stride = stride;

  D3D11_BUFFER_DESC desc = {};
  desc.ByteWidth = bufferByteCount;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA subData = {};
  subData.pSysMem = bufferData;
  // SysMemPitch( unused ):
  //   The distance (in bytes) from the beginning of one line of a texture to the next line
  //   Only used for 2d & 3d textures
  // SysMemSlicePitch( unused ):
  //   The distance (in bytes) from the beginning of one depth level to the next
  //   Only used for 3d textures
  HRESULT hr = device->CreateBuffer( &desc, &subData, &result.buffer );
  if( FAILED( hr ) )
    HandleErrorGracefully();

  return result;
}

void Graphics::SetVertexBuffer( VertexBuffer vertexBuffer )
{
  const UINT bufferCount = 1;
  // One stride value for each buffer in the vertex-buffer array.
  // Each stride is the size (in bytes) of the elements that are to be used from that vertex buffer
  //
  // Question: Isn't it the number of bytes to advance to get from one element in the array
  //   to the next?
  UINT strides[bufferCount] = { vertexBuffer.stride };
  UINT offsets[bufferCount] = { 0 };
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476456(v=vs.85).aspx
  immediateContext->IASetVertexBuffers(
    0,
    bufferCount,
    &vertexBuffer.buffer,
    strides,
    offsets );
}

void Graphics::FreeVertexBuffer( VertexBuffer vertexBuffer )
{
  vertexBuffer.buffer->Release();
}

IndexBuffer Graphics::CreateIndexBuffer(
  void* bufferData,
  UINT bufferByteCount,
  Format format,
  UINT indexCount )
{
  IndexBuffer result;
  result.format = ToDXGI_Format( format );
  result.indexCount = indexCount;

  D3D11_BUFFER_DESC desc = {};
  desc.ByteWidth = bufferByteCount;
  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

  D3D11_SUBRESOURCE_DATA subData = {};
  subData.pSysMem = bufferData;

  HRESULT hr = device->CreateBuffer( &desc, &subData, &result.buffer );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  return result;
}

void Graphics::SetIndexBuffer( IndexBuffer indexBuffer )
{
  immediateContext->IASetIndexBuffer( indexBuffer.buffer, indexBuffer.format, 0 );
}

void Graphics::FreeIndexBuffer( IndexBuffer indexBuffer )
{
  indexBuffer.buffer->Release();
}

void Graphics::Draw( IndexBuffer indexBuffer )
{
  immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
  immediateContext->DrawIndexed( indexBuffer.indexCount, 0, 0 );
}

Texture Graphics::CreateTexture(
  void* bytes,
  int width,
  int height,
  Format format,
  int stride ) // number of bytes from one row to the next
{
  Texture result;
  result.width = width;
  result.height = height;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.ArraySize = 1;
  desc.MipLevels = 1;
  desc.SampleDesc.Count = 1;
  desc.Width = width;
  desc.Height = height;
  desc.Format = ToDXGI_Format( format );
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  // SysMemPitch: Distance in bytes(?) between any two adjacent pixels on different lines.
  // SysMemSlicePitch: Size of the entire 2D surface in bytes.
  D3D11_SUBRESOURCE_DATA data = {};
  data.pSysMem = bytes;
  data.SysMemPitch = stride;
  data.SysMemSlicePitch = stride * height;
  HRESULT hr = device->CreateTexture2D( &desc, &data, &result.texture );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  device->CreateShaderResourceView( result.texture, nullptr, &result.srv );
  return result;
}

void Graphics::FreeTexture( Texture texture )
{
  texture.texture->Release();
  texture.srv->Release();
}

void Graphics::SetTexture( Texture texture, int index )
{
  immediateContext->PSSetShaderResources( index, 1, &texture.srv );
}

ConstantBuffer Graphics::CreateConstantBuffer( UINT bufferSize )
{
  ConstantBuffer result;
  D3D11_BUFFER_DESC desc = {};
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.ByteWidth = bufferSize;
  HRESULT hr = device->CreateBuffer( &desc, nullptr, &result.buffer );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  return result;
}
void Graphics::SetConstantBufferData(
  ConstantBuffer constantBuffer,
  void* data )
{
  immediateContext->UpdateSubresource( constantBuffer.buffer, 0, nullptr, data, 0, 0 );
}
void Graphics::SetConstantBuffer(
  ConstantBuffer constantBuffer,
  UINT slotIndex )
{
  immediateContext->PSSetConstantBuffers( slotIndex, 1, &constantBuffer.buffer );
  immediateContext->VSSetConstantBuffers( slotIndex, 1, &constantBuffer.buffer );
}
void Graphics::FreeConstantBuffer( ConstantBuffer constantBuffer )
{
  constantBuffer.buffer->Release();
}


Blend Graphics::CreateBlend()
{
  Blend result;
  D3D11_BLEND_DESC desc = {};

  D3D11_RENDER_TARGET_BLEND_DESC& rtDesc = desc.RenderTarget[0];
  rtDesc.BlendEnable = TRUE;
  rtDesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
  // color
  rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
  rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
  rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  // alpha
  rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
  rtDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
  rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;

  HRESULT hr = device->CreateBlendState( &desc, &result.state );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  return result;
}
void Graphics::SetBlend( Blend blend )
{
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476462(v=vs.85).aspx
  immediateContext->OMSetBlendState( blend.state, nullptr, 0xffffffff );
}
void Graphics::FreeBlend( Blend blend )
{
  blend.state->Release();
}

Depth Graphics::CreateDepth()
{
  Depth depth;
  D3D11_DEPTH_STENCIL_DESC desc = {};
  desc.DepthEnable = false;

  device->CreateDepthStencilState( &desc, &depth.state );
    return depth;
}
void Graphics::SetDepth( Depth depth )
{
  immediateContext->OMSetDepthStencilState( depth.state, 1 );
}
void Graphics::FreeDepth( Depth depth )
{
  depth.state->Release();
}

Sampler Graphics::CreateSampler()
{
  Sampler result;
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  HRESULT hr = device->CreateSamplerState( &sampDesc, &result.state );
  if( FAILED( hr ) )
    HandleErrorGracefully();
  return result;

}
void Graphics::SetSampler( Sampler sampler, int slot )
{
  immediateContext->PSSetSamplers( slot, 1, &sampler.state );
}
void Graphics::FreeSampler( Sampler sampler )
{
  sampler.state->Release();
}
