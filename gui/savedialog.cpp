#include "savedialog.h"

#include <dirent.h>

#include <Tempest/Layout>
#include "panel.h"
#include "scroolwidget.h"
#include "button.h"
#include "lineedit.h"

#include "gen/ui_loaddialog.h"

#ifndef __ANDROID__
#include <windows.h>
#endif

const std::wstring SaveDialog::sDir = L"./save/";

static bool FileExists( const wchar_t* szPath) {
#ifndef __ANDROID__
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
  return 0;
#endif
  }

struct SaveDialog::Btn:public Button {
  Btn( SaveDialog & s, Resource & res ):Button(res), dlg(s){
    clicked.bind( *this, &Btn::clickedAction );
    }

  void clickedAction(){
    dlg.edit->setText( text() );
    dlg.edit->setSelectionBounds( dlg.edit->text().size(),
                                  dlg.edit->text().size()+1 );
    }

  SaveDialog & dlg;
  };

SaveDialog::SaveDialog(Resource &res, Widget *w)
  : ModalWindow(res,w), res(res){
  layout().add( new Widget() );
  Panel *p = new Panel(res);
  UI::LoadDialog menu;
  menu.setupUi(p, res);
  p->layout().setMargin(8);

  setLayout( Tempest::Vertical );
  p->setMaximumSize(300, 400);
  p->setSizePolicy( Tempest::FixedMax );

  layout().add(p);
  layout().add( new Widget() );
  layout().add( new Widget() );

  menu.topWidget->setMaximumSize( menu.topPanel->sizePolicy().maxSize.w,
                                 30 );
  menu.topWidget->setSizePolicy( menu.topPanel->sizePolicy().typeH,
                                Tempest::FixedMax );

  menu.bottom->setMaximumSize( menu.bottom->sizePolicy().maxSize.w,
                               30 );
  menu.bottom->setSizePolicy( menu.bottom->sizePolicy().typeH,
                              Tempest::FixedMax );
  menu.topWidget->layout().setMargin(0);

  menu.inputName->setFocus(1);
  menu.inputName->setSelectionBounds( menu.inputName->text().size(),
                                      menu.inputName->text().size()+1 );
  edit = menu.inputName;

  menu.cancel->clicked.bind( *this, &SaveDialog::deleteLater );
  menu.accept->clicked.bind( *this, &SaveDialog::acceptAction );
  acceptBtn = menu.accept;

  items = menu.items;

  setDir( sDir );
  }

void SaveDialog::setSaveMode() {
  acceptBtn->setText(L"Сохранить");
  }

void SaveDialog::acceptAction() {
  std::wstring fname = edit->text();

  if( fname.find(L'.')==std::wstring::npos &&
      !FileExists( fname.c_str() ) ){
    fname+=L".sav";
    }

  accept( sDir+fname );
  deleteLater();
  }

std::vector<std::wstring> SaveDialog::filesInDir( const std::wstring &dirName ){
  std::vector<std::wstring> vec;
  (void)dirName;

#ifndef __ANDROID__
  _WDIR *dir = _wopendir ( dirName.c_str() );//data->curDir;
  struct _wdirent *ent;
  
  //dir = _wopendir ( dirName.c_str() );
  if (dir != NULL) {
    /* print all the files and directories within directory */
    while ((ent = _wreaddir (dir)) != NULL) {
      vec.push_back( ent->d_name );
      }
    _wclosedir (dir);
    } else {
    //return EXIT_FAILURE;
    }
#endif

  return vec;
  }

void SaveDialog::setDir(const std::wstring &dirName) {
  items->centralWidget().layout().removeAll();

  auto files = filesInDir( dirName );
  for( size_t i=0; i<files.size(); ++i ){
    if( files[i]!=L"." &&
        files[i]!=L".." ){
      Button *b = new Btn(*this, res);
      b->setText( files[i] );

      items->centralWidget().layout().add( b );
      }
    }

  edit->setText(L"");
  }
