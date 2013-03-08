#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include "modalwindow.h"

#include <string>
#include <vector>

class LineEdit;
class Button;
class ScroolWidget;

class SaveDialog : public ModalWindow {
  public:
    SaveDialog( Resource & res, Tempest::Widget* );

    Tempest::signal< const std::wstring& > accept;

    void setSaveMode();
  private:
    void acceptAction();

    std::vector<std::wstring> filesInDir(const std::wstring& dirName);

    struct Btn;

    LineEdit * edit;
    Button * acceptBtn;
    ScroolWidget * items;

    Resource & res;

    void setDir(const std::wstring& dirName);
  };

#endif // SAVEDIALOG_H
