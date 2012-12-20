#ifndef NATIVESAVEDIALOG_H
#define NATIVESAVEDIALOG_H

#include <string>

class NativeSaveDialog {
  public:
    NativeSaveDialog();

    bool save();
    bool load();

    const std::wstring fileName() const;
  private:
    std::wstring fname;
  };

#endif // NATIVESAVEDIALOG_H
