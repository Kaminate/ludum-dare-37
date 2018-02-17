#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <cmath>
void HandleErrorGracefully( const char* message = nullptr );
#define Unused( parameter ) ( void )parameter;
#define InvalidCodePath HandleErrorGracefully( "Invalid Code Path" );
#define InvalidDefaultCase default: HandleErrorGracefully( "Invalid Default Case" );
#define Assert( exp ) if( !( exp ) ) HandleErrorGracefully();
#define AssertMsg( exp, msg ) if( !exp ) HandleErrorGracefully( msg );
#define ArraySize( array ) sizeof( array ) / sizeof( array[ 0 ] )

enum TacKey
{
  Up,
  Down,
  Left,
  Right,
  Jump,
  Interact,
  Debug,
  Menu,

  Count,
};

struct Input
{
  Input( float windowWidth, float windowHeight )
  {
    width = windowWidth;
    height = windowHeight;
  }
  bool IsKeyJustPressed( TacKey key )
  {
    return IsKeyDownCurr( key ) && !IsKeyDownPrev( key );
  }
  bool IsKeyJustReleased( TacKey key )
  {
    return !IsKeyDownCurr( key ) && IsKeyDownPrev( key );
  }
  bool IsKeyDownCurr( TacKey key )
  {
    return keysDownCurr.find( key ) != keysDownCurr.end();
  }
  bool IsKeyDownPrev( TacKey key )
  {
    return keysDownPrev.find( key ) != keysDownPrev.end();
  }
  double mElapsedSeconds = 0;
  float dt = 1 / 60.0f;
  bool mQuitGameRequested = false;
  float width;
  float height;
  std::set< TacKey > keysDownCurr;
  std::set< TacKey > keysDownPrev;
};

struct Vector2
{
  Vector2(){ x = y = 0; };
  Vector2( float x, float y );
  Vector2 operator-();
  Vector2& operator += ( Vector2 rhs );
  Vector2& operator *= ( float scale );
  Vector2& operator /= ( float scale );
  Vector2 operator/ ( float scale );
  bool IsZero();
  float Length();
  float& operator[]( int index );
  float x;
  float y;
};
float Dot( Vector2 lhs, Vector2 rhs );
Vector2 operator*( float scale, Vector2 vector );
Vector2 operator*( Vector2 vector, float scale );
struct Vector3
{
  Vector3(){};
  Vector3( float x, float y, float z );
  Vector3 operator-();
  float& operator[]( int index );
  float x;
  float y;
  float z;
};
struct Vector4
{
  Vector4(){};
  Vector4( float x, float y, float z, float w );
  Vector4 operator -();
  Vector4 operator *( float scale );
  Vector4 operator /( float scale );
  float& operator[]( int index );
  float x;
  float y;
  float z;
  float w;
};

struct Color4
{
  Color4(){};
  Color4( float red, float green, float blue, float alpha );
  float& operator[]( int index );
  float r;
  float g;
  float b;
  float a;
};

// NOTE:
//   HLSL assumes column major unless otherwise specified with
//   #pragma pack_matrix( row_major )
struct Matrix2
{
  Matrix2(){};
  Matrix2(
    float m00, float m01,
    float m10, float m11 );
  static Matrix2 Identity();
  static Matrix2 Scale( float value );
  static Matrix2 Scale( float scaleX, float scaleY );
  static Matrix2 Rotate( float radians );
  static Matrix2 Rotate( float cos, float sin );
  float* operator[]( int index );
  Matrix2 operator*( Matrix2 rhs );
  float values[ 2 * 2 ];
};
struct Matrix3
{
  Matrix3(){};
  Matrix3( Matrix2 m );
  Matrix3(
    float m00, float m01, float m02,
    float m10, float m11, float m12,
    float m20, float m21, float m22 );
  static Matrix3 Identity();
  float* operator[]( int index );
  Matrix3 operator*( Matrix3 rhs );
  float values[ 3 * 3 ];
};
struct Matrix4
{
  Matrix4(){};
  Matrix4( Matrix2 matrix );
  Matrix4( Matrix3 matrix );
  Matrix4(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33 );
  static Matrix4 Translate( float x, float y );
  static Matrix4 Translate( Vector2 pos );
  static Matrix4 Identity();
  float* operator[]( int index );
  Matrix4 operator*( Matrix4 rhs );
  float values[ 4 * 4 ];
};

Vector4 operator*( Matrix4 lhs, Vector4 rhs );
Vector4 operator*( Vector4 lhs, Matrix4 rhs );

template< typename T >
T Square( T t )
{
  return t * t;
}

// December 12, 2016
// The most beautiful struct I've ever written
struct TemporaryMemory
{
  TemporaryMemory( const char* path );
  TemporaryMemory( unsigned byteCount );
  ~TemporaryMemory();
  char* mBytes;
  unsigned mByteCount;
};

const char* va( const char* format, ... );
const char* boolToString( bool b );
