#ifndef MAPEDIT_H
#define MAPEDIT_H

#include<cmath>
#include <QMainWindow>
#include<fstream>
#include<QLabel>
#include<QPixmap>
#include<QPaintEvent>
#include<QPainter>
#include<QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class Mapedit;
}
QT_END_NAMESPACE

class Mapedit : public QMainWindow
{
    Q_OBJECT

public:
    explicit Mapedit(QWidget *parent = nullptr);

    ~Mapedit();
    QLabel *pic; //暂时没啥用
    QString piclo; //导入地图图片的地址，在构造函数里用到
    //void draw();
    int pichri,piccol; //类全局变量，存储图片在界面上的标准型大小（实际大小不重要，只看在界面上的大小）
    int pichriused,piccolused; //存储图片当前大小
    int valuehri,valuecol; //图片缩放和移动：存储滑块位置便于统一修改
    float i; //缩放图片的缩放系数（坐标变换倍数），类全局变量，非请勿动
    float hrimov; //水平移动系数（坐标变换偏移量），类全局变量，非请勿动
    float colmov; //竖直移动系数（坐标变换偏移量），类全局变量，非请勿动
    int butcas; //定义按钮对应的事件编号，用于在回调函数中进行操作
    int pointNum; //定义点的数量
    int pointLabel; //定义点的标号，用于导入点名称
    double proportion; //地图比例尺，标准型与实际，后端预留
    int speed=80; //速度，算时间
    bool walk=true; //是走吗？
    int miceNum=0; //鼠标计数器
    QPoint mouseMov[2]; //用于鼠标操作
    bool hri=false; //水平滑块标志位
    bool col=false; //竖直滑块标志位
    struct routeSet
    {
        int pot;
        routeSet *next=NULL;
    }; //最短路径点
    float shortestWay=0;

    //内存不足警告，被迫降级
    //求最短路径用的
    bool checkMatrix[90][90][90]; //布尔量检查矩阵，查看是否经过该点位，前两维是首末点，第三维是过路点，true就是过路
    double routeMatrix[90][90]; //路径矩阵，表示两点间距离对应的时间

    //邻接表使用起来实在麻烦，考虑使用邻接矩阵全面代替之，矩阵动态联编不好搞，直接按最大的来（当前方案）
    //以空间换方便程度
    float adjacencyMatrix[90][90]; //邻接矩阵，可用于计算最短路径，邻接表的替代品，预留，计算最短路径再说

    double dist(QPoint a, QPoint b);
    bool isin(QPoint x,QPoint y,int n=5);
    float distInPro(int a, int b);
    double passTime(float route);
    void openmap(QString filename);
    void shortestRoute(int p,int q);
    void toClear();
    void verify(); //距离修正器
    void selectLine(int p,int q); //二期工程：新开发：画最短路径并显示之

    //点坐标变量放在这里，是类成员公共变量，用于之后的调用
    struct myPoint //结构体，存储我们的点，现在里面含有点和点名称
    {
        QPoint P; //点，因为是实际采集点，目前是形式主体（摆设？），主要信息在转换坐标上
        int PN; //点编号
        QString PS; //点名称
        float Px; //点横坐标，由于做了图像缩放存在坐标变换，我们需要一个转换后的坐标
        float Py; //点纵坐标，由于做了图像缩放存在坐标变换，我们需要一个转换后的坐标
        // neighbor *neighborPoint=NULL; //这是我们的邻接表指针。感觉有点怪。不确定，再看看？相当于头指针。
        bool selected=false; //是否点被选中？布尔量，yes or no。建议是即变即走
        bool hidden=false; //隐藏，新增属性
        //原先试图构建邻接表，但不能用已有的点否则会出错，必须重建一个一样的点，占存储空间很大，而且遍历不方便（？）
    }mySetPoint[91]; //最后一天临时修改：满点状态下删点会有问题，必须多一个空位，修改为91


private slots:
    void on_pushButton_clicked(); //点击退出界面

    void paintEvent(QPaintEvent* event); //绘图回调函数，用update函数可触发

    void mousePressEvent(QMouseEvent* event); //鼠标事件回调函数，鼠标点击触发

    void wheelEvent(QWheelEvent *event); //鼠标滑轮效果

    void mouseMoveEvent(QMouseEvent *event); //鼠标拖动效果

    void mouseReleaseEvent(QMouseEvent *event);

    void on_pushButton_2_clicked();//换张图片显示，图片自选默认路径可更改

    void on_horizontalSlider_valueChanged(int value); //缩放图像回调函数（含坐标变换）

    void on_horizontalSlider_2_valueChanged(int value); //水平移动图像回调函数（含坐标变换）

    void on_verticalSlider_valueChanged(int value); //竖直移动图像回调函数（含坐标变换）

    void on_pushButton_3_clicked(); //地图操作，添加一个点

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_18_clicked();

private:
    Ui::Mapedit *ui;
};
#endif // MAPEDIT_H
