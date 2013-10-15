#include <Tempest/Application>
#include "game.h"
#include "lang/lang.h"

#ifdef __WIN32
#ifndef STRICT
#define STRICT
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tlhelp32.h>

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

void setupTh(){
  DWORD mainTh = GetMainThreadId();
  SetThreadAffinityMask( (void*)mainTh, 0);
  }
#else
void setupTh(){}
#endif

int main( int, char** ){
  /*
  Tempest::Pixmap pm("data/textures/land/grass1.png");
  pm.setFormat( Tempest::Pixmap::Format_DXT1 );
  pm.save("data/textures/land/t.dds");
  pm.setFormat( Tempest::Pixmap::Format_RGB );
  pm.save("data/textures/land/t.png");*/

  Lang::load("lang/l.lang", L"rus");
  Tempest::Application app;
  setupTh();
  Game game( Tempest::Window::Maximized );
  game.show();

  return app.exec();
  } 
