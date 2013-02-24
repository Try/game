#include "game.h"
#include <MyWidget/Event>

#ifndef STRICT
#define STRICT
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tlhelp32.h>

#include <string>

#include <iostream>
#include <memory>
#include <exception>

#include "lang/lang.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Game * mgl_demo;
HWND g_hWnd;
bool isAppActive = true, resizeIntent = false;
int winW = 0, winH = 0;

void toogleFullScreen( bool isFullScreen ){
  int w,h;

  if( isFullScreen ){
    DEVMODE mode;
    EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
    w = mode.dmPelsWidth;
    h = mode.dmPelsHeight;

    SetWindowLong( g_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST );
    SetWindowLong( g_hWnd, GWL_STYLE,   WS_POPUP );
    } else {
    w = 800;
    h = 600;

    SetWindowLong( g_hWnd, GWL_EXSTYLE, 0 );
    SetWindowLong( g_hWnd, GWL_STYLE,   WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE );
    }

  SetWindowPos( g_hWnd, 0,
                0, 0, w, h, SWP_SHOWWINDOW );

  if( isFullScreen )
    ShowWindow( g_hWnd, SW_NORMAL ); else
    ShowWindow( g_hWnd, SW_MAXIMIZE );
  UpdateWindow( g_hWnd );

  if( isFullScreen )
    mgl_demo->resizeEvent( w, h );
  }

void exitGame(){
  PostQuitMessage(0);
  }

DWORD GetMainThreadId () {
  const std::shared_ptr<void> hThreadSnapshot(
      CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0), CloseHandle);

  if (hThreadSnapshot.get() == INVALID_HANDLE_VALUE) {
    ;//throw std::runtime_error("GetMainThreadId failed");
    return 0;
    }

  THREADENTRY32 tEntry;
  tEntry.dwSize = sizeof(THREADENTRY32);
  DWORD result = 0;
  DWORD currentPID = GetCurrentProcessId();

  for (BOOL success = Thread32First(hThreadSnapshot.get(), &tEntry);
      !result && success && GetLastError() != ERROR_NO_MORE_FILES;
      success = Thread32Next(hThreadSnapshot.get(), &tEntry)) {
    if (tEntry.th32OwnerProcessID == currentPID) {
      result = tEntry.th32ThreadID;
      }
    }

  return result;
  }

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE /*hPrevInstance*/,
                    LPSTR     /*lpCmdLine*/,
                    int       /*nCmdShow*/ ) {
    Lang::load("./lang/ru.lang");

    WNDCLASSEX winClass; 
    MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

    static std::wstring wclass = L"game";

    winClass.lpszClassName = wclass.data();
    winClass.cbSize        = sizeof(WNDCLASSEX);
    winClass.style         = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc   = WindowProc;
    winClass.hInstance     = hInstance;
    winClass.hIcon         = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(32512));
    winClass.hIconSm       = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(32512));
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;

    if( !RegisterClassEx(&winClass) )
        return E_FAIL;

    /*try*/
    {

    bool isFullScreen = 1;

    int w = GetSystemMetrics(SM_CXFULLSCREEN),
        h = GetSystemMetrics(SM_CYFULLSCREEN);
    DEVMODE mode;
    EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
    w = mode.dmPelsWidth;
    h = mode.dmPelsHeight;

    DWORD style = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;

    if( isFullScreen )
      style = WS_POPUP;

    g_hWnd = CreateWindowEx( NULL, wclass.data(),
                             wclass.data(),
                             style,
                             0, 0,
                             w,
                             h,
                             NULL, NULL, hInstance, NULL );

    if( g_hWnd == NULL )
        return E_FAIL;

    ShowWindow( g_hWnd, isFullScreen ? SW_NORMAL : SW_MAXIMIZE );
    UpdateWindow( g_hWnd );

    RECT rect;
    GetWindowRect( g_hWnd, &rect );
    ClipCursor( &rect );

    {
      RECT rectWindow;
      GetClientRect( HWND(g_hWnd), &rectWindow);

      Game demo( g_hWnd,
                 rectWindow.right-rectWindow.left,
                 rectWindow.bottom-rectWindow.left,
                 isFullScreen );

      mgl_demo = &demo;

      {
        LPWSTR *args;
        int argc = 0;
        args = CommandLineToArgvW(GetCommandLine(), &argc );

        if( argc>1 && std::wstring(args[1])==L"-s" )
          demo.setupAsServer();

        if( argc>2 && std::wstring(args[1])==L"-c" )
          demo.setupAsClient( std::wstring(args[2]) );
        }

      demo.toogleFullScreen.bind( toogleFullScreen );
      demo.exitGame.bind( exitGame );

      DWORD mainTh = GetMainThreadId();
      SetThreadAffinityMask( (void*)mainTh, 0);

      DWORD time = GetTickCount(), gameTime = 0, tickCount = 0;
      const DWORD frameTime = 1000/35;

      while( uMsg.message != WM_QUIT ) {
        if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ) {
          TranslateMessage( &uMsg );
          DispatchMessage( &uMsg );
          } else {
          DWORD ft = GetTickCount();

          if( isAppActive ){
            if( resizeIntent ){
              if( winW*winH ){
                mgl_demo->resizeEvent( winW, winH );
                resizeIntent = false;
                }
              }
            demo.render( GetTickCount() );
            }

          gameTime += std::min<DWORD>((GetTickCount() - time), 3*frameTime);
          time = GetTickCount();

          if( gameTime < tickCount*frameTime )
            gameTime = tickCount*frameTime;

          DWORD tc = (gameTime-tickCount*frameTime)/frameTime;
          //std::cout << "tc = " << tc << std::endl;

          //tc = std::max<DWORD>(1, tc);
          tickCount += tc;

          for( DWORD i=0; i<tc; ++i )
            demo.tick();

          ft  = GetTickCount() - ft;

          //if( ft< 1000/60 )
            //Sleep(1000/60-ft); else
          Sleep(0);
          }
        }

      mgl_demo = 0;
      }

    }/*
    catch(...){
      std::cout << "err";
      }*/

    UnregisterClass( wclass.data(), winClass.hInstance );

    return uMsg.wParam;
}

MyWidget::KeyEvent makeKeyEvent( WPARAM k,
                                 bool scut = false ){
  MyWidget::KeyEvent::KeyType e = MyWidget::KeyEvent::K_NoKey;

  if( k==VK_ESCAPE )
    e = MyWidget::KeyEvent::K_ESCAPE;//PostQuitMessage(0);

  if( k==VK_BACK ){
    e = MyWidget::KeyEvent::K_Back;
    }

  if( k==VK_DELETE ){
    e = MyWidget::KeyEvent::K_Delete;
    }

  if( k==VK_RETURN ){
    e = MyWidget::KeyEvent::K_Return;
    }

  if( k>=VK_LEFT && k<=VK_DOWN )
    e = MyWidget::KeyEvent::KeyType( size_t(MyWidget::KeyEvent::K_Left) + size_t(k) - VK_LEFT );

  if( k>=VK_F1 && k<= VK_F24 )
    e = MyWidget::KeyEvent::KeyType( size_t(MyWidget::KeyEvent::K_F1) + size_t(k) - VK_F1 );

  if( scut ){
    if( k>=0x41 && k<=0x5A )
      e = MyWidget::KeyEvent::KeyType( size_t(MyWidget::KeyEvent::K_A) + size_t(k) - 0x41 );

    if( k>=0x30 && k<=0x39 )
      e = MyWidget::KeyEvent::KeyType( size_t(MyWidget::KeyEvent::K_0) + size_t(k) - 0x30 );
    }

  return MyWidget::KeyEvent(e);
  }

MyWidget::Event::MouseButton toButton( UINT msg ){
  if( msg==WM_LBUTTONDOWN ||
      msg==WM_LBUTTONUP )
    return MyWidget::Event::ButtonLeft;

  if( msg==WM_RBUTTONDOWN  ||
      msg==WM_RBUTTONUP)
    return MyWidget::Event::ButtonRight;

  if( msg==WM_MBUTTONDOWN ||
      msg==WM_MBUTTONUP )
    return MyWidget::Event::ButtonMid;

  return MyWidget::Event::ButtonNone;
  }

LRESULT CALLBACK WindowProc( HWND   hWnd, 
                             UINT   msg, 
                             WPARAM wParam, 
                             LPARAM lParam )
{
    if( !mgl_demo )
      return DefWindowProc( hWnd, msg, wParam, lParam );

    switch( msg )
    {
        case WM_CHAR:
        {
           MyWidget::KeyEvent e = MyWidget::KeyEvent( uint16_t(wParam) );

           DWORD wrd[3] = {
             VK_RETURN,
             VK_BACK,
             0
             };

           if( 0 == *std::find( wrd, wrd+2, wParam) ){
             mgl_demo->keyDownEvent( e );
             mgl_demo->keyUpEvent( e );
             }
           //std::cout << "wParaam = " << wParam << std::endl;
        }
        break;

        case WM_KEYDOWN:
        {
           MyWidget::KeyEvent sce =  makeKeyEvent(wParam, true);
           mgl_demo->scutEvent(sce);

           if( !sce.isAccepted() ){
             MyWidget::KeyEvent e =  makeKeyEvent(wParam);
             if( e.key!=MyWidget::KeyEvent::K_NoKey )
               mgl_demo->keyDownEvent( e );
             }
           //std::cout << "wParaam = " << wParam << std::endl;
        }
        break;

        case WM_KEYUP:
        {
           MyWidget::KeyEvent e =  makeKeyEvent(wParam);
           mgl_demo->keyUpEvent( e );
        }
        break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
          MyWidget::MouseEvent e( LOWORD (lParam),
                                  HIWORD (lParam),
                                  toButton(msg) );
          mgl_demo->mouseDownEvent(e);
        }
        break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
          MyWidget::MouseEvent e( LOWORD (lParam),
                                  HIWORD (lParam),
                                  toButton(msg) );
          mgl_demo->mouseUpEvent(e);
        }
        break;

        case WM_SIZE:{
            winW = LOWORD (lParam),
            winH = HIWORD (lParam);

            if( mgl_demo && isAppActive ){
              RECT rectWindow;
              GetWindowRect( hWnd, &rectWindow);

              if( winW*winH ){
                mgl_demo->resizeEvent( winW, winH );
                resizeIntent = false;
                }
              } else {
              resizeIntent = true;
              }
            }
        break;

        case WM_MOUSEWHEEL:{
          POINT p;
          p.x = LOWORD (lParam);
          p.y = HIWORD (lParam);

          ScreenToClient(hWnd, &p);

          MyWidget::MouseEvent e( p.x, p.y,
                                  MyWidget::Event::ButtonNone,
                                  GET_WHEEL_DELTA_WPARAM(wParam) );
          mgl_demo->mouseWheelEvent(e);
          }
        break;

        case WM_MOUSEMOVE:
        {
          MyWidget::MouseEvent e( LOWORD (lParam),
                                  HIWORD (lParam),
                                  MyWidget::Event::ButtonNone );
          mgl_demo->mouseMoveEvent(e);
        }
        break;

        case WM_ACTIVATEAPP:
        {
            isAppActive = (wParam==TRUE);

            if( !isAppActive && mgl_demo->isFullScr() )
              ShowWindow(g_hWnd, SW_MINIMIZE);
        }
        break;

        case WM_CLOSE:
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;

        default:
        {
            return DefWindowProc( hWnd, msg, wParam, lParam );
        }
        break;
    }

    return 0;
}
