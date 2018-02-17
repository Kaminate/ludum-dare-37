#include "utility.h"
#include "platform.h"
#include <stdarg.h> // vsprintf 

void HandleErrorGracefully( const char* message )
{
  if( message )
    PlatformMessageBox( message );
  else
    std::cout << "Unknown error!" << std::endl;
#if _DEBUG
  __debugbreak();
#endif
  exit( -1 );
}

// is this legit?
Vector4::Vector4( float x, float y, float z, float w ) :x( x ), y( y ), z( z ), w( w ) { }

Vector4 Vector4::operator*( float scale )
{
  return Vector4( x * scale, y * scale, z * scale, w * scale );
}

Vector4 Vector4::operator/( float scale )
{
  return Vector4( x / scale, y / scale, z / scale, w / scale );
}

float& Vector4::operator[]( int index )
{
  return *( &x + index );

}

Vector4 Vector4::operator-()
{
  return Vector4( -x, -y, -z, -w );
}

Vector3::Vector3( float x, float y, float z ) : x( x ), y( y ), z( z ){ }

Vector3 Vector3::operator-()
{
  return Vector3( -x, -y, -z );
}

float& Vector3::operator[]( int index )
{
  return *( &x + index );
}

Vector2::Vector2( float x, float y ) : x( x ), y( y ){ }

Vector2 Vector2::operator-()
{
  return Vector2( -x, -y );
}

Vector2& Vector2::operator*=( float scale )
{
  x *= scale;
  y *= scale;
  return *this;
}

Vector2& Vector2::operator/=( float scale )
{
  x /= scale;
  y /= scale;
  return *this;
}

Vector2 Vector2::operator/( float scale )
{
  return Vector2( x / scale, y / scale );
}

bool Vector2::IsZero()
{
  return x == 0 && y == 0;
}

float Dot( Vector2 lhs, Vector2 rhs )
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

float Vector2::Length()
{
  return std::sqrt( Square( x ) + Square( y ) );
}

float& Vector2::operator[]( int index )
{
  return *( &x + index );
}

Vector2& Vector2::operator+=( Vector2 rhs )
{
  x += rhs.x;
  y += rhs.y;
  return *this;
}

Vector2 operator*( float scale, Vector2 vector )
{
  return Vector2(
    scale * vector.x,
    scale * vector.y );
}

Vector2 operator*( Vector2 vector, float scale )
{
  return Vector2(
    scale * vector.x,
    scale * vector.y );
}


Color4::Color4( float red, float green, float blue, float alpha )
{
  r = red;
  g = green;
  b = blue;
  a = alpha;
}

float& Color4::operator[]( int index )
{
  return *( &r + index );

}

template< typename T, int N >
T MatrixMultiply( T lhs, T rhs )
{
  T result;
  for( int r = 0; r < N; ++r )
  {
    for( int c = 0; c < N; ++c )
    {
      float dot = 0;
      for( int i = 0; i < N; ++i )
      {
        dot += lhs[ r ][ i ] * rhs[ i ][ c ];
      }
      result[ r ][ c ] = dot;
    }
  }
  return result;
}

Matrix2::Matrix2(
  float m00, float m01,
  float m10, float m11 )
{
  values[ 0 ] = m00;
  values[ 1 ] = m01;
  values[ 2 ] = m10;
  values[ 3 ] = m11;
}

Matrix2 Matrix2::Rotate( float radians )
{
  float cos = std::cos( radians );
  float sin = std::sin( radians );
  return Rotate( cos, sin );
}

Matrix2 Matrix2::Rotate( float cos, float sin )
{
  return Matrix2(
    cos, -sin,
    sin, cos );
}

float* Matrix2::operator[]( int index )
{
  return values + index * 2;
}

Matrix2 Matrix2::operator*( Matrix2 rhs )
{
  return MatrixMultiply< Matrix2, 2 >( *this, rhs );
}

Matrix2 Matrix2::Identity()
{
  return Matrix2(
    1, 0,
    0, 1 );
}

Matrix2 Matrix2::Scale( float value )
{
  return Matrix2(
    value, 0,
    0, value );
}

Matrix2 Matrix2::Scale( float scaleX, float scaleY )
{
  return Matrix2(
    scaleX, 0,
    0, scaleY );
}


Matrix3::Matrix3(
  float m00, float m01, float m02,
  float m10, float m11, float m12,
  float m20, float m21, float m22 )
{
  values[ 0 ] = m00;
  values[ 1 ] = m01;
  values[ 2 ] = m02;
  values[ 3 ] = m10;
  values[ 4 ] = m11;
  values[ 5 ] = m12;
  values[ 6 ] = m20;
  values[ 7 ] = m21;
  values[ 8 ] = m22;
}

Matrix3::Matrix3( Matrix2 m )
{
  *this = Matrix3(
    m[ 0 ][ 0 ], m[ 0 ][ 1 ], 0.0f,
    m[ 1 ][ 0 ], m[ 1 ][ 1 ], 0.0f,
    0.0f, 0.0f, 1.0f );
}

Matrix3 Matrix3::Identity()
{
  return Matrix3(
    1, 0, 0,
    0, 1, 0,
    0, 0, 1 );
}



float* Matrix3::operator[]( int index )
{
  return values + 3 * index;
}

Matrix3 Matrix3::operator*( Matrix3 rhs )
{
  return MatrixMultiply< Matrix3, 3 >( *this, rhs );
}

Matrix4::Matrix4(
  float m00, float m01, float m02, float m03,
  float m10, float m11, float m12, float m13,
  float m20, float m21, float m22, float m23,
  float m30, float m31, float m32, float m33 )
{
  int index = 0;
  values[ index++ ] = m00;
  values[ index++ ] = m01;
  values[ index++ ] = m02;
  values[ index++ ] = m03;

  values[ index++ ] = m10;
  values[ index++ ] = m11;
  values[ index++ ] = m12;
  values[ index++ ] = m13;

  values[ index++ ] = m20;
  values[ index++ ] = m21;
  values[ index++ ] = m22;
  values[ index++ ] = m23;

  values[ index++ ] = m30;
  values[ index++ ] = m31;
  values[ index++ ] = m32;
  values[ index++ ] = m33;
}

Matrix4::Matrix4( Matrix2 matrix )
{
  *this = Matrix4(
    matrix[ 0 ][ 0 ], matrix[ 0 ][ 1 ], 0, 0,
    matrix[ 1 ][ 0 ], matrix[ 1 ][ 1 ], 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1 );
}

Matrix4::Matrix4( Matrix3 matrix )
{
  *this = Matrix4(
    matrix[ 0 ][ 0 ], matrix[ 0 ][ 1 ], matrix[ 0 ][ 1 ], 0,
    matrix[ 1 ][ 0 ], matrix[ 1 ][ 1 ], matrix[ 1 ][ 2 ], 0,
    matrix[ 2 ][ 0 ], matrix[ 2 ][ 1 ], matrix[ 2 ][ 2 ], 0,
    0, 0, 0, 1 );
}

float* Matrix4::operator[]( int index )
{
  return values + index * 4;
}
Matrix4 Matrix4::Identity()
{
  return Matrix4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1 );
}

Matrix4 Matrix4::operator*( Matrix4 rhs )
{
  return MatrixMultiply< Matrix4, 4 >( *this, rhs );
}
Matrix4 Matrix4::Translate( float x, float y )
{
  return Matrix4(
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, 0,
    0, 0, 0, 1 );
}
Matrix4 Matrix4::Translate( Vector2 pos )
{
  return Translate( pos.x, pos.y );
}

Vector4 operator*( Matrix4 lhs, Vector4 rhs )
{
  int N = 4;
  Vector4 result;
  for( int r = 0; r < N; ++r )
  {
    float dot = 0;
    for( int i = 0; i < N; ++i )
    {
      dot += lhs[ r ][ i ] * rhs[ i ];
    }
    result[ r ] = dot;
  }
  return result;
}
Vector4 operator*( Vector4 lhs, Matrix4 rhs )
{
  Vector4 result;
  int N = 4;
  for( int c = 0; c < N; ++c )
  {
    float dot = 0;
    for( int i = 0; i < N; ++i )
    {
      dot += lhs[ i ] * rhs[ i ][ c ];
    }
    result[ c ] = dot;
  }
  return result;
}

TemporaryMemory::TemporaryMemory( unsigned byteCount )
{
  mBytes = new char[ mByteCount = byteCount ];
}

TemporaryMemory::TemporaryMemory( const char* path )
{
  std::ifstream ifs( path, std::ifstream::binary );
  if( !ifs.is_open() )
    HandleErrorGracefully( path );
  ifs.seekg( 0, ifs.end );
  mBytes = new char[ mByteCount = ( int )ifs.tellg() ];
  ifs.seekg( 0, ifs.beg );
  ifs.read( mBytes, mByteCount );
  ifs.close();
}

TemporaryMemory::~TemporaryMemory()
{
  delete[] mBytes;
}

const char* va( const char* format, ... )
{
  static char buffer[ 512 ];
  va_list args;
  va_start( args, format );
  vsprintf_s( buffer, format, args );
  va_end( args );
  return buffer;
}

const char* boolToString( bool b )
{
  return b ? "true" : "false";
}

