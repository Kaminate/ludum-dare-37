#include "game.h"
#include "windows_platform.h"
#include <chrono>

//typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;
//typedef std::chrono::duration< float > TimeDelta;

Input* input;

LRESULT CALLBACK MyWindowProc(
  HWND   hWnd,
  UINT   msg,
  WPARAM wParam,
  LPARAM lParam )
{
  LRESULT result = 0;
  switch( msg )
  {
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      BeginPaint( hWnd, &ps );
      EndPaint( hWnd, &ps );
    } break;
    case WM_CLOSE:
    case WM_QUIT:
    {
      input->mQuitGameRequested = true;
    } break;
    case WM_MOVE: break;
    case WM_SIZE: break;
    case WM_SETFOCUS:
    break;
    case WM_KILLFOCUS:
    {
      input->keysDownCurr.clear();
    } break;
    break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
      std::map< WPARAM, TacKey > keyMap;
      keyMap[ 'W' ] = TacKey::Up;
      keyMap[ 'A' ] = TacKey::Left;
      keyMap[ 'S' ] = TacKey::Down;
      keyMap[ 'D' ] = TacKey::Right;
      keyMap[ VK_UP ] = TacKey::Up;
      keyMap[ VK_LEFT ] = TacKey::Left;
      keyMap[ VK_DOWN ] = TacKey::Down;
      keyMap[ VK_RIGHT ] = TacKey::Right;
      keyMap[ VK_SPACE ] = TacKey::Jump;
      keyMap[ 'E' ] = TacKey::Interact;
      keyMap[ VK_F1 ] = TacKey::Debug;
      keyMap[ VK_ESCAPE ] = TacKey::Menu;

      bool isKeyDown = ( lParam & ( 1 << 31 ) ) == 0;
      Assert( ( msg == WM_KEYDOWN && isKeyDown )
        || ( msg == WM_KEYUP && !isKeyDown ) );

      auto it = keyMap.find( wParam );
      if( it == keyMap.end() )
        break;

      TacKey key = ( *it ).second;
      if( isKeyDown )
        input->keysDownCurr.insert( key );
      else
        input->keysDownCurr.erase( key );

      OutputDebugString( va( "%c %s\n",
        ( char )wParam,
        isKeyDown ? "pressed" : "released" ) );

      VK_LBUTTON; // left mouse
      VK_RBUTTON; // right mouse
      VK_RETURN; // enter
      VK_SHIFT;
      VK_CONTROL;
      VK_MENU; // alt
      VK_ESCAPE;
      VK_DELETE;
      'A';
      '0';
      VK_LSHIFT;
      VK_RSHIFT;
      VK_LCONTROL;
      VK_RCONTROL;
      VK_PACKET; // ???
    } break;

    default:
    {
      result = DefWindowProc( hWnd, msg, wParam, lParam );
    }
  }
  return result;
}

int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nCmdShow )
{
  Unused( lpCmdLine );
  Unused( hPrevInstance );
  // TODO: remove the L?
  auto className = "my_class_name";
  auto windowName = "One Room";
  //"Studio Sleeping";

  WNDCLASSA wndclass = {};
  wndclass.lpszClassName = className;
  wndclass.hInstance = hInstance;
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = MyWindowProc;
  // Without this, the app uses the "busy..." cursor
  wndclass.hCursor = LoadCursor( NULL, IDC_ARROW );

  auto registerClassResult = RegisterClass( &wndclass );
  if( registerClassResult == 0 )
    HandleErrorGracefully();

  int x = 50;
  int y = 50;
  int clientwidth = 1000;
  int clientheight = 500;


  RECT wr = { 0, 0, clientwidth, clientheight };
  AdjustWindowRect( &wr, WS_OVERLAPPEDWINDOW, FALSE );

  HWND windowHandle = CreateWindow(
    className,
    windowName,
    WS_TILEDWINDOW,
    x,
    y,
    wr.right - wr.left,
    wr.bottom - wr.top,
    nullptr,
    nullptr,
    hInstance,
    nullptr );
  if( windowHandle == nullptr )
  {
    HandleErrorGracefully();
  }

  input = new Input( ( float )clientwidth, ( float )clientheight );
  Graphics* graphics = new Graphics( windowHandle, clientwidth, clientheight );
  Game* game = new Game( graphics, input );
  ShowWindow( windowHandle, nCmdShow );
  MSG Msg;
  auto mLastTime = std::chrono::high_resolution_clock::now();
  while( !input->mQuitGameRequested )
  {
    while( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
    {
      TranslateMessage( &Msg );
      DispatchMessage( &Msg );
    }

    auto currTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration< float > diff = currTime - mLastTime;
    if( diff < std::chrono::duration< float >( input->dt ) )
      continue;
    input->mElapsedSeconds += input->dt;
    mLastTime = currTime;
    game->Update();
    input->keysDownPrev = input->keysDownCurr;
  }
  delete input;
  delete game;
  delete graphics;
  return 0;
}
