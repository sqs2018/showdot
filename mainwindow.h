#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <interactiveview.h>
#include "resizer/graphicsitemresizer.h"
#include <QGraphicsRectItem>



typedef struct GERBERDATA
{
    QString partName;
    double X;
    double Y;
}GerberData;


typedef struct NewRect
{
   int left;
   int top;
   int right;
   int bottom;
}NewRect;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QList<GerberData>  getGerberData();
    //点是否在括号内
    bool containsPoint(NewRect rect,QPointF point);
    //获取可变动框的坐标位置
    NewRect getRect();


public slots:
     void queryData();
     //写出文件
     void writeFile();



private:
    Ui::MainWindow *ui;
    InteractiveView* view;
    QGraphicsScene * scene;
    QList<GerberData> dataList;
    QList<QGraphicsEllipseItem*> dotList;//点的集合
    QGraphicsRectItem* rectItem;//框
    GraphicsItemResizer *resizer;//改变大小的


};

#endif // MAINWINDOW_H
