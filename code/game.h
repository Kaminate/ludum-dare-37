#pragma once
#include "graphics.h"

#include "stb_truetype.h"

struct ConstantBufferData
{
  Matrix4 world;
  Matrix4 view;
  Color4 color;
  Vector2 uvMin;
  Vector2 uvMax;
};
struct Game
{
  Game( Graphics* graphics, Input* input );
  void Update();
  ~Game();

  Input* mInput;
  Graphics* mGraphics;

  Shader mSpriteShader;
  Shader mTextShader;
  InputLayout mInputLayout;
  VertexBuffer mVertexBuffer;
  IndexBuffer mIndexBuffer;
  ConstantBuffer mConstantBuffer;
  Depth mDepth;
  Blend mBlend;
  Sampler mSampler;
  Texture mStar;
  Texture mHachicro;

  float mTextScale;
  Vector2 mTextPosition;

  Vector2 characterPosition;
  Vector2 characterVelocity;


  float mFontSize;
  stbtt_fontinfo fontinfo;
  stbtt_packedchar packedchars[ 128 ];
  void RenderEnd( ConstantBufferData constantBufferData );
};