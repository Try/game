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

    struct PImpl;
  };

#endif // NATIVESAVEDIALOG_H
