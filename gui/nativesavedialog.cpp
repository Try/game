#include "nativesavedialog.h"

#ifndef __ANDROID__
#include <windows.h>
#endif

struct NativeSaveDialog::PImpl {
#ifndef __ANDROID__
  static HWND getHwnd(){
    return FindWindow(L"game", L"game");
    }
#endif
  };

NativeSaveDialog::NativeSaveDialog() {

  }

bool NativeSaveDialog::load() {
#ifndef __ANDROID__
  WCHAR fileName[MAX_PATH] = {};
  OPENFILENAME of;
  ZeroMemory( &of, sizeof(of) );

  of.lStructSize       = sizeof(OPENFILENAME);
  of.hwndOwner         = PImpl::getHwnd();
  of.hInstance         = 0;
  of.lpstrFilter       = L"All files (*.*)\0*.*\0";
  of.lpstrCustomFilter = NULL;
  of.nMaxCustFilter    = 0;
  of.nFilterIndex      = 1;
  of.lpstrFile         = fileName;
  of.nMaxFile          = MAX_PATH;
  of.lpstrFileTitle    = NULL;
  of.nMaxFileTitle     = 0;
  of.lpstrInitialDir   = NULL;
  of.Flags             = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

  TCHAR dir[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, dir);

  if (GetOpenFileName(&of)) {
    fname = fileName;
    SetCurrentDirectory(dir);
    return 1;
    }

  SetCurrentDirectory(dir);
#endif
  return 0;
  }

const std::wstring NativeSaveDialog::fileName() const {
  return fname;
  }

bool NativeSaveDialog::save() {
#ifndef __ANDROID__
  WCHAR fileName[MAX_PATH] = {};
  OPENFILENAME of;
  ZeroMemory( &of, sizeof(of) );

  of.lStructSize       = sizeof(OPENFILENAME);
  of.hwndOwner         = PImpl::getHwnd();
  of.hInstance         = 0;
  of.lpstrFilter       = L"All files (*.*)\0*.*\0";
  of.lpstrCustomFilter = NULL;
  of.nMaxCustFilter    = 0;
  of.nFilterIndex      = 1;
  of.lpstrFile         = fileName;
  of.nMaxFile          = MAX_PATH;
  of.lpstrFileTitle    = NULL;
  of.nMaxFileTitle     = 0;
  of.lpstrInitialDir   = NULL;
  of.Flags             = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

  TCHAR dir[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, dir);

  if (GetSaveFileName(&of)) {
    fname = fileName;
    SetCurrentDirectory(dir);
    return 1;
    }

  SetCurrentDirectory(dir);
#endif
  return 0;
  }
