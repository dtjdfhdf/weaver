#include <QtGui/QApplication>
#include "weavermanualalign.h"

int main(int argc, char *argv[])
{
    int appendImgRelXPos, appendImgRelYPos;

    QApplication a(argc, argv);

    // specify the two tiles to load and the appendImgRelXPos and appendImgRelYPos
    // will contian the relative position of the appendImg as picked by the user
    WeaverManualAlign w(NULL);
    w.show(WM_REF_IMG_FILE, WM_APPEND_IMG_FILE, _WSM_VERTICAL,
                        &appendImgRelXPos, &appendImgRelYPos);
    //w.show();
    return a.exec();
}
