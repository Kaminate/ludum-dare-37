#include "windows_platform.h"

void PlatformMessageBox( const char* msg )
{
  MessageBox( nullptr, msg, nullptr, MB_OK );
}
