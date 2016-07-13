#include "interface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Interface w;



    w.show();


    //номализация рразмера окна приложения при запуске
    int width_window = 0;
    int height_window = 0;
    QRect old_geometry = w.geometry();
    if(old_geometry.width() > old_geometry.height()){
        width_window = 800;
        height_window = 600;
    }
    else {
        width_window = 600;
        height_window = 800;
    }
    w.setGeometry(old_geometry.x(), old_geometry.y(), width_window, height_window);

    return a.exec();
}
