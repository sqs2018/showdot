#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ctime>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDir>

#define GW 800
#define GH 800


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    view=ui->graphicsView;
    scene= new QGraphicsScene();
    view->setScene(scene);

    qsrand(time(NULL));


    //改变大小的框
    rectItem = new QGraphicsRectItem(QRectF(0, 0, 50, 50));
    rectItem->setPos(10, 10);
    rectItem->setFlag(QGraphicsItem::ItemIsMovable);
    rectItem->setPen(QColor(255, 255, 0));
    rectItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //item->setBrush(QColor(158, 204, 255));
    scene->addItem(rectItem);

    resizer = new GraphicsItemResizer(rectItem);
    resizer->setBrush(QColor(64, 64, 64));
    resizer->setMinSize(QSizeF(30, 30));
    resizer->setPen(QColor(102, 102, 102));
    resizer->setTargetSize(rectItem->boundingRect().size());
    resizer->setFlag(QGraphicsItem::ItemIgnoresTransformations);

     QGraphicsRectItem* tempItem=rectItem;
    QObject::connect(resizer, &GraphicsItemResizer::targetRectChanged, [tempItem](const QRectF &rect)
    {
        QPointF pos = tempItem->pos();
        tempItem->setPos(pos + rect.topLeft());
        QRectF old = tempItem->rect();
        tempItem->setRect(QRectF(old.topLeft(), rect.size()));
    });
     tempItem->setZValue(1000);
     resizer->setZValue(1000);
     view->myItem=(rectItem);


    //view->setc
    //开启鼠标移动监视
    ui->centralWidget->setMouseTracking(true);
    view->setMouseTracking(true);
    setMouseTracking(true);  //这是激活整个窗体的鼠标追踪

    //设置坐标系
    scene->setSceneRect(-GW/2,-GH/2,GW/2,GH/2);
    //设置x向左方向
    view->scale(-1, 1);




    //读取文件
   dataList= getGerberData();


   for(GerberData data:dataList){

       QString text=QString("%1,%2").arg(data.X).arg(data.Y);//QString::number(data.x,10)+","+QString::number(data.y);
       QGraphicsTextItem *txtitem = new QGraphicsTextItem(text);

       QGraphicsEllipseItem * dot=new QGraphicsEllipseItem();
       dot->setRect(data.X,data.Y,2,2);



       dotList.push_back(dot);
       scene->addItem(dot);


       //scene->addEllipse(int(data.X),int(data.Y),2, 2);
       txtitem->setPos(QPointF(int(data.X), int(data.Y)));//设置要放置的的位置

        //字体反转
       QTransform transform(txtitem->transform());
       transform.rotate(180, Qt::YAxis);
       transform.translate(txtitem->boundingRect().width() * -1, 0);
       txtitem->setTransform(transform);



       scene->addItem(txtitem);

   }



   //画中心点

   QPen pen;
   pen.setWidth(2);
   pen.setColor(QColor(255, 0, 0));
   scene->setBackgroundBrush(QBrush(Qt::gray));
   scene->addLine(-100, 0, 100, 0, pen);
   scene->addLine(0, -100, 0, 100, pen);
   scene->addEllipse(-25, -25, 50, 50, pen);
   //绑定事件
   connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::queryData);
   connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::writeFile);

}

void MainWindow::writeFile(){

    QString curPath=QDir::currentPath().append("/data");//获取系统当前目录
     QDir dir(curPath);
     if(!dir.exists()){
         dir.mkdir(curPath);
     }
     //用 QTextStream打开文本文件
     QFile file(curPath.append("/data.txt"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

     QTextStream aStream(&file); //用文本流读取文件

       //获取点的数据
    QStandardItemModel* model = new QStandardItemModel();
    QStringList labels = QObject::trUtf8("X,Y,值").simplified().split(",");
    model->setHorizontalHeaderLabels(labels);
    NewRect rect=getRect();
    if(resizer){

        int i=0;
        //读取数据
        for(int j=0;j<dataList.size();j++){
            GerberData data=dataList.at(j);
                    QPointF point;
                    point.setX((int)data.X);
                    point.setY((int)data.Y);
                 //在框内
                 if(containsPoint(rect,point)){
                     dotList.at(j)->setBrush(QColor(255, 0, 0));
                     dotList.at(j)->setPen(QColor(255, 0, 0));
                     //定义item
                     QStandardItem* item = 0;
                     item = new QStandardItem(QString("%1").arg(data.X));
                     model->setItem(i,0,item);
                     item = new QStandardItem(QString("%1").arg(data.Y));
                     model->setItem(i,1,item);
                     item = new QStandardItem(QString("%1").arg(i));
                     model->setItem(i,2,item);
                      i++;
                      aStream<<QString("%1").arg(data.X)<<":"<<QString("%1").arg(data.Y)<<"\n";
                 }else{
                     //设置hk为的色为黑色
                     dotList.at(j)->setBrush(QColor(0, 0, 0));
                     dotList.at(j)->setPen(QColor(0, 0, 0));
                 }
        }
    }








      file.close();//关闭文件



}


//点是否在括号内
bool MainWindow::containsPoint(NewRect rect,QPointF point){
    qDebug()<<point.x()<<"<"<<rect.left<<"&&"<<point.x()<<">"<<rect.right;
    if(point.x()<=rect.left&&point.x()>=rect.right){
        if(point.y()<=rect.bottom&&point.y()>=rect.top){
            return true;
        }
    }
    return false;
}

NewRect MainWindow::getRect(){
    QPointF point=resizer->mapToScene( QPointF(resizer->x(),resizer->y()));
    QSizeF size(resizer->boundingRect().width(),resizer->boundingRect().height());
    QRectF rect(point,size);
    //修正后的rect
    NewRect newRect;

    newRect.left=(int(rect.left()+10));
    newRect.top=(int(rect.top()+10));
    newRect.right=(int(rect.right()-rect.width()*2+15));
    newRect.bottom=(int(rect.bottom()));

  // qDebug()<<rect.left()<<":"<<rect.top()<<":"<<rect.right()<<":"<<rect.bottom();
   //scene->addEllipse(int(rect.left()+10),int(rect.top()+10),2, 2);
   //scene->addEllipse(int(rect.right()-rect.width()*2+15),int(rect.bottom()),2, 2);
    //qDebug()<<newRect;
   return newRect;
}

//获取框内数据
void MainWindow::queryData(){

    QStandardItemModel* model = new QStandardItemModel();
    QStringList labels = QObject::trUtf8("X,Y,值").simplified().split(",");
    model->setHorizontalHeaderLabels(labels);
    NewRect rect=getRect();
    if(resizer){

        int i=0;
        //读取数据
        for(int j=0;j<dataList.size();j++){
            GerberData data=dataList.at(j);
                    QPointF point;
                    point.setX((int)data.X);
                    point.setY((int)data.Y);
                 //在框内
                 if(containsPoint(rect,point)){
                     dotList.at(j)->setBrush(QColor(255, 0, 0));
                     dotList.at(j)->setPen(QColor(255, 0, 0));
                     //定义item
                     QStandardItem* item = 0;
                     item = new QStandardItem(QString("%1").arg(data.X));
                     model->setItem(i,0,item);
                     item = new QStandardItem(QString("%1").arg(data.Y));
                     model->setItem(i,1,item);
                     item = new QStandardItem(QString("%1").arg(i));
                     model->setItem(i,2,item);
                      i++;
                 }else{
                     //设置hk为的色为黑色
                     dotList.at(j)->setBrush(QColor(0, 0, 0));
                     dotList.at(j)->setPen(QColor(0, 0, 0));
                 }
        }
    }

    ui->tableView->setModel(model);
    ui->tableView->show();

}





QList<GerberData>  MainWindow::getGerberData()
{
    try
    {
        QList<GerberData> m_gerberData;
        QString gerberIniPath = QApplication::applicationDirPath() +"/gerber.ini";
        if (QFile::exists(gerberIniPath))
        {
            QSettings settings(gerberIniPath, QSettings::IniFormat);
            try
            {
                int num = settings.allKeys().count();
                for (int i = 0;i<num/2;i++)
                {
                    GerberData tempData;
                    tempData.X =QString::number(settings.value(QString("/gerber/%1").arg("X%1").arg(i+1), \
                                                               tempData.X).toString().toDouble(),'f',2).toDouble();
                    tempData.Y = QString::number(settings.value(QString("/gerber/%1").arg("Y%1").arg(i+1), \
                                                                 tempData.Y).toString().toDouble(),'f',2).toDouble();
                    m_gerberData.append(tempData);
                }
                return m_gerberData;
            }
            catch (...)
            {
                throw;
            }
        }
        return m_gerberData;
    }
    catch (...)
    {
        throw;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
