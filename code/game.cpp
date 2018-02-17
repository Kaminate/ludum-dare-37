#include "game.h"

#define ENABLE_GAME_INPUT_DEBUG 0

#pragma warning( push )  
// unreferenced formal parameter
#pragma warning( disable : 4100 )
// conversion from 'foo' to 'bar', possible loss of data
#pragma warning( disable : 4244 )
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning( pop )  

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Vertex
{
  Vector3 pos;
  Vector2 uv;
};


Game::Game( Graphics* graphics, Input* input ) : mGraphics( graphics ), mInput( input )
{
  characterPosition = Vector2( 0, 3 );
  mTextPosition = Vector2( 3, 3 );
  mTextScale = 4;

  // Font
  {
    std::memset( &fontinfo, 0, sizeof( fontinfo ) );
    TemporaryMemory fontFileMemory( "data/kenvector_future_thin.ttf" );
    //TemporaryMemory fontFileMemory( "data/letter_cases.ttf" );
    stbtt_InitFont( &fontinfo, ( unsigned char* )fontFileMemory.mBytes, 0 );
    int fontAtlasWidth = 400;
    int fontAtlasHeight = 400;
    TemporaryMemory fontAtlasCPU( fontAtlasWidth * fontAtlasHeight );
    float fontSizeInitial = 30;
    int state = 0;
    for( int iterationCount = 0; state != 2; ++iterationCount )
    {
      mFontSize = fontSizeInitial + iterationCount * 0.1f;
      if( state == 1 )
        state = 2;
      stbtt_pack_context spc = {};
      if( !stbtt_PackBegin(
        &spc,
        ( unsigned char* )fontAtlasCPU.mBytes,
        fontAtlasWidth,
        fontAtlasHeight,
        0,
        1,
        nullptr ) )
        HandleErrorGracefully();
      if( !stbtt_PackFontRange(
        &spc,
        ( unsigned char* )fontFileMemory.mBytes,
        0,
        mFontSize,
        0,
        128,
        packedchars ) )
      {
        if( !iterationCount )
          HandleErrorGracefully();
        state = 1;
        iterationCount -= 2;
      }
      stbtt_PackEnd( &spc );
    }
    mHachicro = mGraphics->CreateTexture(
      fontAtlasCPU.mBytes,
      fontAtlasWidth,
      fontAtlasHeight,
      Format::r8unorm,
      fontAtlasWidth );
  }

  // Star
  {
    TemporaryMemory memory( "data/star.png" );
    int x;
    int y;
    int channels;
    void* pixels = stbi_load_from_memory(
      ( stbi_uc* )memory.mBytes,
      memory.mByteCount,
      &x,
      &y,
      &channels,
      4 );
    Assert( pixels );
    mStar = graphics->CreateTexture(
      pixels,
      x,
      y,
      Format::r8g8b8a8unorm,
      4 * x );
    stbi_image_free( pixels );
  }

  // Graphics creation
  {
    mSpriteShader = mGraphics->LoadShader( "data/sprite.fx" );
    mTextShader = mGraphics->LoadShader( "data/text.fx" );
    mBlend = mGraphics->CreateBlend();
    mDepth = mGraphics->CreateDepth();
    mSampler = mGraphics->CreateSampler();

    LayoutCreator layoutCreator;
    layoutCreator.AddLayout( "POSITION", Format::r32g32b32float );
    layoutCreator.AddLayout( "TEXCOORD", Format::r32g32float );
    mInputLayout = mGraphics->CreateInputLayout( layoutCreator, mSpriteShader );

    // 3---2
    // | \ |
    // 0---1
    Vertex ndcTriVerts[ 4 ];

    Vertex& v0 = ndcTriVerts[ 0 ];
    v0.pos = Vector3( -1, -1, 0 );
    v0.uv = Vector2( 0, 1 );

    Vertex& v1 = ndcTriVerts[ 1 ];
    v1.pos = Vector3( 1, -1, 0 );
    v1.uv = Vector2( 1, 1 );

    Vertex& v2 = ndcTriVerts[ 2 ];
    v2.pos = Vector3( 1, 1, 0 );
    v2.uv = Vector2( 1, 0 );

    Vertex& v3 = ndcTriVerts[ 3 ];
    v3.pos = Vector3( -1, 1, 0 );
    v3.uv = Vector2( 0, 0 );

    mVertexBuffer = mGraphics->CreateVertexBuffer(
      ndcTriVerts,
      sizeof( ndcTriVerts ),
      sizeof( Vertex ) );

    // in directx, front face default winding is clockwise

    uint16_t indexes[] = {
      0, 3, 1, // bottom left
      1, 3, 2 }; // top right
    const UINT indexCount = ArraySize( indexes );
    mIndexBuffer = mGraphics->CreateIndexBuffer(
      indexes,
      sizeof( indexes ),
      Format::r16uint,
      indexCount );

    mConstantBuffer = mGraphics->CreateConstantBuffer( sizeof( ConstantBufferData ) );
  }

  // Graphics state
  {
    mGraphics->SetConstantBuffer( mConstantBuffer, 0 );
    mGraphics->SetIndexBuffer( mIndexBuffer );
    mGraphics->SetVertexBuffer( mVertexBuffer );
    mGraphics->SetBlend( mBlend );
    mGraphics->SetDepth( mDepth );
    mGraphics->SetSampler( mSampler, 0 );
    mGraphics->SetInputLayout( mInputLayout );
  }
}

void Game::RenderEnd( ConstantBufferData constantBufferData )
{
  mGraphics->SetConstantBufferData( mConstantBuffer, &constantBufferData );
  mGraphics->Draw( mIndexBuffer );
}

void Game::Update()
{
  Vector2 uvMin( 0, 0 );
  Vector2 uvMax( 1, 1 );
  Backbuffer backbuffer = mGraphics->GetBackbuffer();
  mGraphics->SetRenderTarget( backbuffer );
  mGraphics->Clear( backbuffer, Color4( 1, 0.5f, 0, 1 ) );
  mGraphics->SetViewport( mInput->width, mInput->height );

  mGraphics->SetShader( mSpriteShader );
  mGraphics->SetTexture( mStar, 0 );

  ConstantBufferData constantBufferData = {};
  constantBufferData.uvMin = uvMin;
  constantBufferData.uvMax = uvMax;
  constantBufferData.color = Color4(
    124 / 255.0f,
    186 / 255.0f,
    91 / 255.0f,
    1.0f );

  // update character with input 
  {
    Vector2 inputDir( 0, 0 );
    std::map< TacKey, Vector2 > keyDir;
    keyDir[ TacKey::Right ] = Vector2( 1, 0 );
    keyDir[ TacKey::Left ] = Vector2( -1, 0 );
    keyDir[ TacKey::Up ] = Vector2( 0, 1 );
    keyDir[ TacKey::Down ] = Vector2( 0, -1 );
    for( auto pair : keyDir )
      inputDir += pair.second * mInput->IsKeyDownCurr( pair.first );

    float maxSpeed = 10;
    float accel = 30.0f;
    Vector2 accelVec = accel * inputDir;

    float curSpeed = characterVelocity.Length();
    if( inputDir.IsZero() && curSpeed > 0.1f )
      accelVec += -( characterVelocity / curSpeed ) * 20;

    characterVelocity += accelVec * mInput->dt;
    characterPosition += characterVelocity * mInput->dt;

    if( curSpeed > maxSpeed )
      characterVelocity *= maxSpeed / curSpeed;

    characterVelocity *= 0.99f;

  }


  Matrix4 world;
  {
    float characterRadius = 5.0f
      + 0.05f * ( float )std::sin( mInput->mElapsedSeconds * 1.0f );
    float temp = ( float )std::sin( mInput->mElapsedSeconds * 5.0f );
    temp *= ( float )std::sin( mInput->mElapsedSeconds * 4.0f + 2.0f );
    float characterRotation = 0;// 1.1f * temp;
    Matrix2 characterscale = Matrix2::Scale( characterRadius );
    Matrix2 characterrotation = Matrix2::Rotate( characterRotation );
    Matrix4 charactertranslation = Matrix4::Translate( characterPosition );
    world = charactertranslation * characterrotation * characterscale;
  }
  constantBufferData.world = world;


  Matrix4 view;
  {
    float aspectRatio = mInput->width / mInput->height;
    float cameraRotation = 0.0f;
    float cameraWidth = 10.0f;
    float cameraHeight = cameraWidth / aspectRatio;
    Vector2 cameraPosition( 0, 0 );
    Matrix2 camerascale = Matrix2::Scale( 1.0f / cameraWidth, 1.0f / cameraHeight );
    Matrix2 camerarotation = Matrix2::Rotate( -cameraRotation );
    Matrix4 cameratranslation = Matrix4::Translate( -cameraPosition );
    view = Matrix4( camerascale * camerarotation ) * cameratranslation;
  }
  constantBufferData.view = view;

  RenderEnd( constantBufferData );

  ///////////////
  // DRAW TEXT //
  ///////////////

  mGraphics->SetShader( mTextShader );
  mGraphics->SetTexture( mHachicro, 0 );
  std::string phrase = "AaZz@$";
  static int phraseCharIndex = 0;
  //for( char c : phrase )
  {
    char c = phrase[ phraseCharIndex ];
    stbtt_packedchar packedChar = packedchars[ c ];
    int width = ( int )packedChar.x1 - ( int )packedChar.x0;
    int height = ( int )packedChar.y1 - ( int )packedChar.y0;
    Unused( width );
    Unused( height );
    packedChar.xadvance;

    //STBTT_DEF void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing)
    int ascent;
    int descent;
    int linegap;
    stbtt_GetFontVMetrics( &fontinfo, &ascent, &descent, &linegap );
    float scale = stbtt_ScaleForPixelHeight( &fontinfo, mFontSize );
    float verticalSpacning = ( ascent - descent + linegap ) * scale;

    Unused( ascent );
    Unused( descent );
    Unused( linegap );
    Unused( verticalSpacning );

    float worldUnits_to_fontSize = 30.0f;
    Unused( worldUnits_to_fontSize );

    if( mInput->IsKeyJustPressed( TacKey::Interact ) )
      phraseCharIndex = ( phraseCharIndex + 1 ) % phrase.size();

    static bool d;
    if( mInput->IsKeyJustPressed( TacKey::Jump ) )
    {
      d = !d;
    }
    if( d )
    {
      uvMin.x = ( float )packedChar.x0 / ( float )mHachicro.width;
      uvMax.x = ( float )packedChar.x1 / ( float )mHachicro.width;
      uvMin.y = ( float )packedChar.y0 / ( float )mHachicro.height;
      uvMax.y = ( float )packedChar.y1 / ( float )mHachicro.height;
    }
  }

  constantBufferData.uvMin = uvMin;
  constantBufferData.uvMax = uvMax;
  constantBufferData.world
    = Matrix4::Translate( mTextPosition )
    * Matrix2::Scale( mTextScale );


  RenderEnd( constantBufferData );

  mGraphics->SwapBuffers();

#if _DEBUG
  if( mInput->IsKeyJustPressed( TacKey::Debug ) )
    __debugbreak();
#endif

  if( mInput->IsKeyJustPressed( TacKey::Menu ) )
    mInput->mQuitGameRequested = true;

#if ENABLE_GAME_INPUT_DEBUG
    OutputDebugString( va( "wasd curr %i%i%i%i prev %i%i%i%i \n",
      mInput->IsKeyDownCurr( TacKey::Up ),
      mInput->IsKeyDownCurr( TacKey::Left ),
      mInput->IsKeyDownCurr( TacKey::Down ),
      mInput->IsKeyDownCurr( TacKey::Right ),
      mInput->IsKeyDownPrev( TacKey::Up ),
      mInput->IsKeyDownPrev( TacKey::Left ),
      mInput->IsKeyDownPrev( TacKey::Down ),
      mInput->IsKeyDownPrev( TacKey::Right ) ) );
#endif
}

Game::~Game()
{
  mGraphics->FreeShader( mSpriteShader );
  mGraphics->FreeIndexBuffer( mIndexBuffer );
  mGraphics->FreeInputLayout( mInputLayout );
  mGraphics->FreeVertexBuffer( mVertexBuffer );
  mGraphics->FreeConstantBuffer( mConstantBuffer );
  mGraphics->FreeTexture( mStar );
  mGraphics->FreeTexture( mHachicro );
  mGraphics->FreeBlend( mBlend );
  mGraphics->FreeDepth( mDepth );
  mGraphics->FreeSampler( mSampler );
}
