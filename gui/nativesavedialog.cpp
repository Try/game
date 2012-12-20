#include "nativesavedialog.h"

#include <windows.h>

NativeSaveDialog::NativeSaveDialog() {

  }

bool NativeSaveDialog::load() {
  WCHAR fileName[MAX_PATH] = {};
  OPENFILENAME of;
  ZeroMemory( &of, sizeof(of) );

  of.lStructSize       = sizeof(OPENFILENAME);
  of.hwndOwner         = 0;
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

  if (GetOpenFileName(&of)) {
    fname = fileName;
    return 1;
    }

  return 0;
  }

const std::wstring NativeSaveDialog::fileName() const {
  return fname;
  }

bool NativeSaveDialog::save() {
  WCHAR fileName[MAX_PATH] = {};
  OPENFILENAME of;
  ZeroMemory( &of, sizeof(of) );

  of.lStructSize       = sizeof(OPENFILENAME);
  of.hwndOwner         = 0;
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

  if (GetSaveFileName(&of)) {
    fname = fileName;
    return 1;
    }

  return 0;
  }
