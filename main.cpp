#include "game.h"
#include <MyWidget/Event>

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>

#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Game * mgl_demo;
HWND g_hWnd;
bool isAppActive = true;

std::wstring toStr( const char* c ){
  std::wstring re;

  for( int i=0; c[i]; ++i )
    re.push_back(c[i]);

  return re;
  }

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

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE /*hPrevInstance*/,
                    LPSTR     /*lpCmdLine*/,
                    int       nCmdShow ) {
    WNDCLASSEX winClass; 
    MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

    static std::wstring wclass = toStr("game");

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

    bool isFullScreen = 0;

    int w = GetSystemMetrics(SM_CXFULLSCREEN),
        h = GetSystemMetrics(SM_CYFULLSCREEN);
    DEVMODE mode;
    EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
    w = mode.dmPelsWidth;
    h = mode.dmPelsHeight;

    DWORD style = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;

    if( isFullScreen )
      style = WS_EX_TOPMOST | WS_POPUP;

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

    {
      RECT rectWindow;
      GetClientRect( HWND(g_hWnd), &rectWindow);

      Game demo( g_hWnd,
                 rectWindow.right-rectWindow.left,
                 rectWindow.bottom-rectWindow.left,
                 isFullScreen );

      mgl_demo = &demo;
      demo.toogleFullScreen.bind( toogleFullScreen );


      DWORD time = GetTickCount(), gameTime = 0, tickCount = 0;
      const DWORD frameTime = 1000/40;

      while( uMsg.message != WM_QUIT ) {
        if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ) {
          TranslateMessage( &uMsg );
          DispatchMessage( &uMsg );
          } else {
          DWORD ft = GetTickCount();

          if( isAppActive )
            demo.render( GetTickCount() );

          gameTime += std::min<DWORD>((GetTickCount() - time), 5*frameTime);
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

          if( ft< 1000/60 )
            Sleep(1000/60-ft);
          }
        }
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
    PostQuitMessage(0);

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

        case WM_SIZE:
            if( mgl_demo && isAppActive ){
              RECT rectWindow;
              GetWindowRect( hWnd, &rectWindow);

              if( LOWORD (lParam)*HIWORD (lParam) ){
                mgl_demo->resizeEvent( LOWORD (lParam),
                                       HIWORD (lParam) );
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
