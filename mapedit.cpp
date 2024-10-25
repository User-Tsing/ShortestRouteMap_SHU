#include "mapedit.h"
#include "ui_mapedit.h"
#include<iostream>
#include<QLabel>
#include<QPixmap>
#include<QPushButton>
#include<QFileDialog>
#include<QFont>
#include<QSize>
#include<QSlider>
#include<QMessageBox>
#include<math.h>
#include<QComboBox>
#include<QDialog>
#include<QVBoxLayout>
#include<selectdialog.h>
#include<QInputDialog>
#include<QColor>
#include<QPalette>


//前端交互式界面：地图及地图最短路径
//directed by STAssn
//算法全部放在大函数里面，不分设小函数因为大多数内容只用一次放哪都一样



Mapedit::Mapedit(QWidget *parent) //构造函数
    : QMainWindow(parent)
    , ui(new Ui::Mapedit)
{
    ui->setupUi(this); //创建界面

    this->setFixedSize(1280,650); //设置界面大小
    piclo="../../Map.jpg"; //地图地址，不同设备时请更改之！！！
    i=4; //缩放之比例初始化，适配设定的边框
    valuehri=50; //水平图片移动系数初始化，bug修复经历：不能直接初始化hrimov，会出错
    valuecol=50; //竖直图片移动系数初始化，bug修复经历：不能直接初始化colmov，会出错
    update(); //触发回调函数paintEvent（一个字不能改）
    butcas=0; //初始化按钮判断符

    pointNum=0;
    pointLabel=0;
    proportion=3.10; //比例尺初始化，指标准型和实际的比例尺，暂定尚未计算，目前测量：3.10

    if(walk==true)
    {
        speed=80; //单位分钟，勿忘
    }
    else
    {
        speed=300; //单位分钟
    }

    for(int j=0;j<90;j++)
    {
        for(int k=0;k<90;k++)
        {
            adjacencyMatrix[j][k]=0; //邻接矩阵初始化：全零
        }
    }
    QString filename = "../../SHUwalkMap.map2";
    //关于为什么是两次../，我认为.exe文件实际上是在build/Desktop_Qt_6_7_2_MSVC2019_64bit-Release中，而非再里面的release文件夹虽然表面上是这样的，如果提取出来那一切照旧
    openmap(filename); //初始加载地图（？）
    verify(); //试图修正

    //字体等的设置
    QFont newFont("Microsoft YaHei", 12);
    QFont titleFont("Microsoft YaHei", 15);
    QFont settedFont("Microsoft YaHei", 8);
    QColor myColour(75,0,130);
    QPalette pal=ui->label_7->palette();
    pal.setColor(QPalette::WindowText,myColour);
    ui->label_7->setPalette(pal);
    ui->label_7->setFont(titleFont);
    QPalette pal2=ui->label_8->palette();
    pal2.setColor(QPalette::WindowText,myColour);
    ui->label_8->setPalette(pal);
    ui->label_8->setFont(settedFont);


    ui->textEdit->setFont(newFont);
    ui->textEdit_2->setFont(newFont);

}

Mapedit::~Mapedit()
{
    delete ui;
}






int lineNum=0; //线的数量,初始为0
struct line //定义结构体：连线（点的结构体在类定义里面），主要用途是画线
{
    int a,b; //两个点都是多少号
    bool minline=false;
    //int routetype=1; //预留，路径类型，现由于骑行步行路径合流，仅在计算时间上有所差异，已成废案
    int routecondi=1; //预留，路况
}lines[180];

void Mapedit::verify()
{
    for(int j=0;j<lineNum;j++)
    {
        adjacencyMatrix[lines[j].a][lines[j].b]=distInPro(lines[j].a,lines[j].b);
        adjacencyMatrix[lines[j].b][lines[j].a]=distInPro(lines[j].a,lines[j].b);

        // adjacencyMatrix[lines[j].a][lines[j].b]=dist(mySetPoint[lines[j].a].P,mySetPoint[lines[j].b].P);
        // adjacencyMatrix[lines[j].b][lines[j].a]=dist(mySetPoint[lines[j].a].P,mySetPoint[lines[j].b].P);
    }
}


double Mapedit::dist(QPoint a, QPoint b)//辅助函数：计算两点间实际距离
{
    return sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()));
}
bool Mapedit::isin(QPoint x, QPoint y, int n)//辅助函数：判断鼠标光标是否点击成功（半径n的圆域范围内）
{
    if (dist(x, y) <= n) return true;
    else return false;
}

float Mapedit::distInPro(int a,int b) //辅助函数：计算点之间带比例尺的距离(其实是时间，距离按比例缩放了)，后端可能调用
{
    float dis=sqrt((mySetPoint[a].Px-mySetPoint[b].Px)*(mySetPoint[a].Px-mySetPoint[b].Px)+(mySetPoint[a].Py-mySetPoint[b].Py)*(mySetPoint[a].Py-mySetPoint[b].Py));
    float routedis = dis*proportion; //此时已是标准型，无需修正！
    return (routedis); //算法，暂定
}

double Mapedit::passTime(float route) //辅助函数：由路径长度计算时间最短者
{
    return(route/speed);
}

void Mapedit::toClear() //辅助函数，清理门户
{
    for(int j=0;j<lineNum;j++)
    {
        lines[j].minline=false;
    }
    for(int j=0;j<pointNum;j++)
    {
        mySetPoint[j].selected=false;
    }
    ui->listWidget->clear();
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    update();
}

void Mapedit::openmap(QString filename) //辅助函数，打开一张带路线的图，计划构造函数中使用，可以一上来就打开地图
{
    //槽函数调用它！
    using namespace std;
    //QString filename = QFileDialog::getOpenFileName(this, "选择文件", "", "地图文件(*.map2)");
    if (filename != "")
    {
        ifstream in(filename.toStdString());
        if (in.is_open())
        {
            string c;
            //in >> c; //文件名
            //QMessageBox::information(this, "提示", "文件名已导出"+QString::fromStdString(c));
            //ui->label_5->setText(QString::fromStdString(c));//转变为Qstring，并显示
            int num; //1,2
            in >> lineNum >> num; //线路数和点数，点数是类成员变量，不能直接赋值，要有一个中继，然而真的是这样吗？
            //QMessageBox::information(this, "提示", "路线数和点数已导出");
            pointNum=num;
            pointLabel=num;
            ui->label_5->setText(QString::number(num));
            //QMessageBox::information(this, "提示", "路线数和点数已导出："+QString::number(num)+"个点");
            for (int j = 0; j < pointNum; j++)
            {
                //QMessageBox::information(this, "提示", "点赋值中");
                float a, b; //点坐标
                in >> a >> b; //3,4
                mySetPoint[j].P.setX(a);
                mySetPoint[j].P.setY(b);
                mySetPoint[j].Px=a;
                mySetPoint[j].Py=b;
                mySetPoint[j].PN=j;
                in >> c; //点名,5
                mySetPoint[j].PS = QString::fromStdString(c);
                in >> mySetPoint[j].hidden; //是否隐藏
                //QMessageBox::information(this, "提示", "导入点：("+QString::number(a)+","+QString::number(b)+")，"+QString::fromStdString(c));
            }
            for (int k = 0; k < pointNum; k++)
            {
                for (int j = 0; j < pointNum; j++)
                {
                    float matrixnum;
                    in >> matrixnum; //6
                    adjacencyMatrix[k][j]=matrixnum;
                }
            }
            for (int k = 0; k < lineNum; k++)
            {
                in >> lines[k].a >> lines[k].b >> lines[k].routecondi; //lines不是类成员变量而是全局变量，可以直接赋值
                //QMessageBox::information(this, "提示", "导出线：点"+QString::number(lines[k].a)+"到点"+QString::number(lines[k].b));
            } //7,8,9
            in >> c; //10
            piclo = QString::fromStdString(c); //图片地址
            //QMessageBox::information(this, "提示", "地图："+QString::fromStdString(c));
            //clr(); //文字栏清屏，未实装
            //QMessageBox::information(this, "提示", "读取成功");
            in.close(); //关闭文件
            //piclo="../../Map.jpg";
            update();
        }
        else QMessageBox::information(this, "提示", "读取失败");
    }
}

Mapedit::routeSet* headpot=new Mapedit::routeSet; //二期工程关键步骤：路径链表，根据需求设立废除

void Mapedit::selectLine(int p,int q) //二期工程新增函数（关键步骤）：标记最短路径。意在优化补充功能但牺牲了时间和空间
{
    //directed by STAssn
    //功能：获取途径点及线路，计算最短路径，标注途经线路
    //思想：弗洛伊德算法计算所有点之间距离，我们从三维矩阵中可以获得路径途径点，然后利用互通时间大小得到途径顺序，用链表记录之
    //由途径顺序获取相应线路标记之，并从邻接矩阵获取线路长度叠加之，即可获得最短路程
    //在后续函数中利用途径点链表顺序输出到达时间
    //目的：解决直连点绕路直线被标记的问题，解决到各点时间乱序输出的问题

    //QMessageBox::information(this,"提示","开始标记路径");
    shortestWay=0; //重要！必须清零路径，否则会一直叠加！
    headpot->pot=p;
    routeSet* x=headpot;
    float min=routeMatrix[p][q];
    int dot=q;
    bool yy=true;
    while(yy==true)
    {
        for(int j=0;j<pointNum;j++)
        {
            if((checkMatrix[p][q][j]==true)&&(j!=p))
            {
                yy=true;
                break;
            }
        }
        for(int j=0;j<pointNum;j++)
        {
            if((checkMatrix[p][q][j]==true)&&(j!=p))
            {
                if(routeMatrix[p][j]<min)
                {
                    min=routeMatrix[p][j]; //从小到大取值
                    //routeMatrix[p][j]=INFINITY; //一次性用品，过河拆桥，但只拆一半因为还要恢复
                    //checkMatrix[p][q][j]=false;
                    dot=j; //保存点序号
                    //QMessageBox::information(this,"提示","比较："+mySetPoint[dot].PS+"距离"+QString::number(min));
                }
                //QMessageBox::information(this,"提示",mySetPoint[j].PS);
            }
        }
        //QMessageBox::information(this,"提示",mySetPoint[dot].PS);
        //routeMatrix[p][dot]=INFINITY; //一次性用品，过河拆桥，但只拆一半因为还要恢复
        //routeSet[counter]=dot;
        checkMatrix[p][q][dot]=false;
        routeSet* xx=new routeSet;
        xx->pot=dot;
        if(headpot->next==NULL)
        {
            headpot->next=xx;
            x=xx;
        }
        else
        {
            x->next=xx;
            x=xx;
        }
        min=routeMatrix[p][q];
        //QMessageBox::information(this,"提示",mySetPoint[dot].PS+":"+QString::number(routeMatrix[p][dot]));
        if(dot==q)
        {
            break;
        }
        dot=q;
    }
    //QMessageBox::information(this,"提示","链表创建好了");
    //两次循环之后应该已存满
    x=headpot;
    while(x->next!=NULL)
    {
        //QMessageBox::information(this,"提示",mySetPoint[x->pot].PS);
        for(int k=0;k<lineNum;k++)
        {
            if((lines[k].a==x->pot)&&(lines[k].b==x->next->pot))
            {
                //有线排到了！
                lines[k].minline=true; //标记之
                //QMessageBox::information(this,"提示",mySetPoint[x->pot].PS+"和"+mySetPoint[x->next->pot].PS);
                shortestWay=shortestWay+adjacencyMatrix[x->pot][x->next->pot]; //最短路程
            }
            else if((lines[k].b==x->pot)&&(lines[k].a==x->next->pot))
            {
                //有线排到了！
                lines[k].minline=true; //标记之
                //QMessageBox::information(this,"提示",mySetPoint[x->pot].PS+"和"+mySetPoint[x->next->pot].PS);
                shortestWay=shortestWay+adjacencyMatrix[x->pot][x->next->pot]; //最短路程
            }
        }
        x=x->next;
    }
    //QMessageBox::information(this,"提示","线路设计好了");
    // for(int j=0;j<counter;j++) //恢复函数恢复路径矩阵，后面还要用
    // {
    //     routeMatrix[p][x->pot]=routeMatrix[x->pot][p];
    //     x=x->next;
    // }
    //QMessageBox::information(this,"提示","路径结算完成");
    //return counter;
}

void Mapedit::shortestRoute(int p,int q) //辅助函数，搜寻最短路径（关键步骤）
{
    //checked by STAssn
    //最短路径：弗洛伊德算法
    //简单测试，可以实现
    //难点在于占内存太大了，在缩圈之前都是直接崩溃了根本无法运行，后续其他功能酌情实现
    //重构邻接矩阵为距离矩阵，直接遍历然后找最短
    //QMessageBox::information(this,"提示","开始求最短路径");
    //static int sta=poi;
    // bool checkMatrix[100][100][100]; //布尔量检查矩阵，查看是否经过该点位，前两维是首末点，第三维是过路点，true就是过路
    // float routeMatrix[100][100]; //路径矩阵，表示两点间距离对应的时间
    //预处理
    for(int j=0;j<pointNum;j++)
    {
        for(int k=0;k<pointNum;k++)
        {
            if((j!=k)&&(adjacencyMatrix[j][k]==0))
            {
                routeMatrix[j][k]=INFINITY; //0取正无穷
            }
            else
            {
                routeMatrix[j][k]=passTime(adjacencyMatrix[j][k]); //其他还一样，但改变成时间尺度（因为要求最短时间而非单纯最小值）
            }
            for(int l=0;l<pointNum;l++)
            {
                checkMatrix[j][k][l]=false; //初始路径都设没有路径
            }
            if(routeMatrix[j][k]<INFINITY) //两点之间有直接路径：检查矩阵中标出
            {
                checkMatrix[j][k][j]=true;
                checkMatrix[j][k][k]=true;
                for(int lin=0;lin<lineNum;lin++) //引入路况信息，待完善，增加了时间复杂度
                {
                    if(((lines[lin].a==j)&&(lines[lin].b==k))||((lines[lin].a==k)&&(lines[lin].b==j)))
                    {
                        if(lines[lin].routecondi==2) //路况尺度造成的影响
                        {
                            routeMatrix[j][k]=routeMatrix[j][k]*2;
                        }
                        else if(lines[lin].routecondi==3)
                        {
                            routeMatrix[j][k]=routeMatrix[j][k]*4;
                        }
                    }
                }
            }
        }
    }
    //QMessageBox::information(this,"提示","初始化完成");
    //计算最短路径
    for(int u=0;u<pointNum;u++)
    {
        for(int v=0;v<pointNum;v++)
        {
            for(int w=0;w<pointNum;w++)
            {
                //全过程遍历，看有没有更短的路径，有就取代之
                if(routeMatrix[v][u]+routeMatrix[u][w]<routeMatrix[v][w]) //还有更短？
                {
                    routeMatrix[v][w]=routeMatrix[v][u]+routeMatrix[u][w]; //数据代入，取而代之
                    for(int k=0;k<pointNum;k++)
                    {
                        checkMatrix[v][w][k]=checkMatrix[v][u][k]||checkMatrix[u][w][k]; //状态导入，中间值u被置1，添加路径点
                    }
                }
            }
        }
    }
    //QMessageBox::information(this,"提示","计算完成，路径时长："+QString::number(routeMatrix[p][q]));
    //理论上至此两个矩阵都已完成，可得最短路径
    if(routeMatrix[p][q]==INFINITY)
    {
        //QMessageBox::warning(this,"警告","两点间不存在路径！");
        return; //失败就退出函数了
    }
    float shortestTime =routeMatrix[p][q];

    selectLine(p,q);
    //QMessageBox::information(this,"提示","路径返回成功");

    //在此，我们采用了更合理的算法画路线，以时间和空间换功能，初有成效，以下成为废案，在此保存遗址

   //  int passPoint=0; //途径点数
   //  int passWay[80]; //途径点
   //  int passNum=0; //计数
   //  //QMessageBox::information(this,"提示","开始算途径点");
   //  for(int k=0;k<pointNum;k++) //提取途径点的点编号
   //  {
   //      if(checkMatrix[p][q][k]==true)
   //      {
   //          passPoint++;
   //          passWay[passNum++]=k;
   //          //QMessageBox::information(this,"提示","沿途经过点"+QString::number(k));
   //      }
   //  }
   // //至此，我们把途径点提取出来了，接下来我们应该去找线，先把途径点中所有的相邻点连起来
   //  float routeDistance=0; //路径长度
   //  for(int k=0;k<passPoint;k++) //先遍历小号途径点
   //  {
   //      for(int j=0;j<lineNum;j++) //每个小号途径点遍历一遍线结构体
   //      {
   //          if(lines[j].a==passWay[k]) //线结构体中有这个小号的线？
   //          {
   //              //QMessageBox::information(this,"提示","找到一个，"+QString::number(k)+" "+QString::number(j));
   //              for(int l=k;l<passPoint;l++) //在小号基础上途径点往上遍历
   //              {
   //                  if(lines[j].b==passWay[l]) //有和这个小号连线的大号？
   //                  {
   //                      lines[j].minline=true; //就是它了，标记出来
   //                      routeDistance=routeDistance+adjacencyMatrix[lines[j].a][lines[j].b]; //计算距离

   //                      //ui->listWidget->addItem("从"+mySetPoint[p].PS+"出发，到达");
   //                      //QMessageBox::information(this,"提示","第"+QString::number(j)+"条线："+mySetPoint[lines[j].a].PS+"到点"+mySetPoint[lines[j].b].PS);
   //                  }
   //              }
   //          }
   //      }
   //  }
   //  shortestWay=routeDistance;
   //  //次关键步骤，重复路去除，明明有近路却在绕路？一定是“路况”的影响！
   //  int tri[20];
   //  int triNum=0;
   //  for(int k=0;k<passPoint;k++) //全局遍历，效率奇低
   //  {
   //      for(int l=0;l<passPoint;l++)
   //      {
   //          for(int m=0;m<passPoint;m++)
   //          {
   //              if((k!=l)&&(k!=m)&&(l!=m)) //重复不算！！！
   //              {
   //                  if((adjacencyMatrix[passWay[k]][passWay[l]]!=0)&&(adjacencyMatrix[passWay[l]][passWay[m]]!=0)&&(adjacencyMatrix[passWay[k]][passWay[m]]!=0)) //组成铁三角？
   //                  {
   //                      if(adjacencyMatrix[passWay[k]][passWay[l]]+adjacencyMatrix[passWay[k]][passWay[m]]>adjacencyMatrix[passWay[l]][passWay[m]])
   //                      {
   //                          //明明路线有最近的为什么要绕路？路况不好，排除之
   //                          //QMessageBox::information(this,"提示","有“铁三角”"+mySetPoint[passWay[k]].PS+" "+mySetPoint[passWay[l]].PS+" "+mySetPoint[passWay[m]].PS);
   //                          //现在待查点是k
   //                          int che=0;
   //                          if((passWay[k]!=p)&&(passWay[k]!=q))
   //                          {
   //                              //不是首末点？

   //                              for(int ck=0;ck<lineNum;ck++)
   //                              {
   //                                  if(((lines[ck].a==passWay[k])||(lines[ck].b==passWay[k])))
   //                                  {
   //                                      //QMessageBox::information(this,"提示","点"+QString::number(lines[ck].a)+"和点"+QString::number(lines[ck].b));
   //                                      if(lines[ck].minline==true)
   //                                      {
   //                                          che++; //看看有几个路径线被选中（即在最短路径上），就两条就是要保留的（即不经过此点的删除），三条就是正线点
   //                                          //QMessageBox::information(this,"提示","选到：点"+QString::number(lines[ck].a)+"和点"+QString::number(lines[ck].b));
   //                                      }
   //                                  }
   //                              }
   //                          }
   //                          //QMessageBox::information(this,"提示","线路数"+QString::number(che));
   //                          if(che==2)
   //                          {
   //                              int tema,temb;
   //                              if(passWay[l]>passWay[m])
   //                              {
   //                                  tema=passWay[m];
   //                                  temb=passWay[l];
   //                              }
   //                              else
   //                              {
   //                                  tema=passWay[l];
   //                                  temb=passWay[m];
   //                              }
   //                              for(int j=0;j<lineNum;j++)
   //                              {
   //                                  if((lines[j].a==tema)&&(lines[j].b==temb))
   //                                  {
   //                                      //找到这根线，看看是不是路况不行取消选中
   //                                      tri[triNum]=j; //保存删线代号，别直接删后面还要遍历
   //                                      triNum++;
   //                                      //lines[j].routecondi=true; //留着一起删
   //                                      //QMessageBox::information(this,"提示","记录"+QString::number(triNum));
   //                                  }
   //                              }
   //                          }
   //                      }
   //                  }
   //              }
   //          }
   //      }
   //  }

   //  //删多余的线
   //  for(int lin=0;lin<triNum;lin++)
   //  {
   //      lines[tri[lin]].minline=false;
   //      //QMessageBox::information(this,"提示","删线"+QString::number(triNum)+"跟");
   //  }



    //线找完了，画图画出来
    //update();
    //时间显示优化
    int minute,second;
    minute=int(shortestTime);
    second=int((shortestTime-minute)*60);
    //QMessageBox::information(this,"提示","返回时间");

    ui->textEdit->setText("从"+mySetPoint[p].PS+"到"+mySetPoint[q].PS+"的最短路径如图所示，最短路程为"+QString::number(shortestWay)+"米,花费时间约为"+QString::number(minute)+"分钟"+QString::number(second)+"秒。");
    ui->textEdit_2->setText("从"+mySetPoint[p].PS+"到"+mySetPoint[q].PS+"的最短路径如图所示，最短路程为"+QString::number(shortestWay)+"米,花费时间约为"+QString::number(minute)+"分钟"+QString::number(second)+"秒。");
    //QMessageBox::information(this,"提示","返回路程");

    //这里显示沿途各点到达时间，不按时间顺序
    // for(int j=0;j<passPoint;j++)
    // {
    //     if(passWay[j]!=p)
    //     {
    //         if(mySetPoint[passWay[j]].hidden==false)
    //         {
    //             //时间显示优化
    //             minute=int(routeMatrix[p][passWay[j]]);
    //             second=int((routeMatrix[p][passWay[j]]-minute)*60);
    //             ui->listWidget->addItem("从"+mySetPoint[p].PS+"到"+mySetPoint[passWay[j]].PS+"约需要"+QString::number(minute)+"分钟"+QString::number(second)+"秒。");
    //         }
    //     }
    // }

    routeSet* x=headpot->next; //在右侧白板上写上到各点的时间
    //QMessageBox::information(this,"提示","准备返回各个点位");
    while(x!=NULL)
    {
        if(mySetPoint[x->pot].hidden==false)
        {
            //时间显示优化
            minute=int(routeMatrix[p][x->pot]);
            second=int((routeMatrix[p][x->pot]-minute)*60);
            ui->listWidget->addItem("从"+mySetPoint[p].PS+"到"+mySetPoint[x->pot].PS+"约需要"+QString::number(minute)+"分钟"+QString::number(second)+"秒。");
        }
        x=x->next;
    }

    routeSet* y=headpot->next; //顺序删除链表
    x=y->next;
    while(x!=NULL)
    {
        delete y;
        y=x;
        x=y->next;
    }
    delete y;

}



void Mapedit::paintEvent(QPaintEvent* event) //在界面上绘图的回调函数，专用，update函数触发，画地图画路线
{
    //directed by STAssn
    //关键步骤：显示图形
    //思想：在界面上画图，实时更新，分类讨论，按图索骥
    //界面大小1280，650
    //由于参考系坐标不变，为确保后续图像缩放和位移时点对点设置距离等的设定与计算能正常使用，采用坐标变换的方式，采用相对坐标（不知道原来有没有这种功能）
    //计划是点坐标由现有实际坐标投射到pic初始坐标（标准型？）上，再进行计算
    //第一步：画地图。这个函数是回调函数，图片的更新在于反复中断来调用该函数重新画图
    QPainter painter(this); //绘图对象设立
    QPixmap pic(piclo); //字符串转Pixmap，用处是把地图.jpg文件导入
    //ui->label_5->setText(QString::number(pointNum)); //测试栏，之后删

    //设置图像边框，注意图片显示
    QSize picsize(pic.size()/i); //取图像大小并取缩放值
    //static QSize picrec=pic.size()/4; //标准型
    QSize picrec=pic.size()/4; //标准型，问题解决了，之前换图大小不变是因为开了static
    pichri=picrec.width(); //存图片大小
    piccol=picrec.height(); //存图片大小
    pichriused=picsize.width(); //存图片大小
    piccolused=picsize.height(); //存图片大小
    QRect clipr(650-picrec.width()/2,300-picrec.height()/2, picrec.width() , picrec.height()); //以(700,300)为中心
    painter.setClipRect(clipr); //设置：超过边界不予显示，用于附带坐标变换的图像缩放
    painter.drawRect(650-picrec.width()/2,300-picrec.height()/2, picrec.width() , picrec.height());//矩形大小，窗口大小

    //画图：地图本图，带上所有的坐标变换
    //相对坐标：确认中心点(650,300)，缩放已由picsize给出，水平移动则移动水平中心点,竖直移动则移动竖直中心点
    //疑似有bug，换其他图片时边框不会适配新图片，而是继续沿用原来的边框，这个问题之后再看看能不能修正
    if(hri==true)//滑块标志控制
    {
        hrimov=(valuehri*1.00/99*2*(pichriused/2-pichri/2)-(pichriused/2-pichri/2))*(-1); //实时读取数据而变化
    }
    if(col==true)
    {
        colmov=valuecol*1.00/99*2*(piccolused/2-piccol/2)-(piccolused/2-piccol/2); //实时读取数据而变化
    }
    if(hrimov<(-pichriused/2+pichri/2)) //图片检测强制控制操作：限定范围不能留白，必须占据整个边框
    {
        hrimov=-pichriused/2+pichri/2; //这个是图片中心水平偏移量，范围是[-(当前图片宽度/2-标准型图片宽度/2),(当前图片宽度/2-标准型图片宽度/2)]
    }
    else if(hrimov>(pichriused/2-pichri/2))
    {
        hrimov=pichriused/2-pichri/2;
    }
    if(colmov<(-piccolused/2+piccol/2))
    {
        colmov=-piccolused/2+piccol/2; //这个是图片中心竖直偏移量，范围是[-(当前图片高度/2-标准型图片高度/2),(当前图片高度/2-标准型图片高度/2)]
    }
    else if(colmov>(piccolused/2-piccol/2))
    {
        colmov=piccolused/2-piccol/2;
    }
    painter.drawPixmap(650+hrimov-picsize.width()/2,300+colmov-picsize.height()/2,picsize.width(),picsize.height(),pic); //图形绘制


    //可能会用到字体，预设一下字体格式
    QFont font1("Microsoft YaHei", 9); //字体：微软雅黑9号
    QFont font2("Microsoft YaHei", 12); //字体：微软雅黑12号
    QColor mycolor(75,0,130);

    painter.setFont(font1);

    painter.setRenderHint(QPainter::Antialiasing, true); //使接下来的绘图光滑
    //重要步骤：画线(待完善)
    QPen Linepen; //定义一个画线笔，可以设置参数
    for(int j=0;j<lineNum;j++)
    {
        float pointXa=(mySetPoint[lines[j].a].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
        float pointYa=(mySetPoint[lines[j].a].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
        QPoint toLineA(pointXa,pointYa);
        float pointXb=(mySetPoint[lines[j].b].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
        float pointYb=(mySetPoint[lines[j].b].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
        QPoint toLineB(pointXb,pointYb);
        if(lines[j].routecondi==1) //路况正常
        {
            Linepen.setWidth(3); //线条粗细
            Linepen.setColor(Qt::green); //绿色的
            painter.setPen(Linepen);
            if(lines[j].minline==false)
            {
                painter.drawLine(toLineA,toLineB);
            }
        }
        else if(lines[j].routecondi==2)
        {
            Linepen.setWidth(3); //线条粗细
            Linepen.setColor(Qt::yellow); //黄色的
            painter.setPen(Linepen);
            if(lines[j].minline==false)
            {
                painter.drawLine(toLineA,toLineB);
            }
        }
        else if(lines[j].routecondi==3)
        {
            Linepen.setWidth(3); //线条粗细
            Linepen.setColor(Qt::red); //红色的
            painter.setPen(Linepen);
            if(lines[j].minline==false)
            {
                painter.drawLine(toLineA,toLineB);
            }
        }
    }

    //分两批画，后画选中的
    for(int j=0;j<lineNum;j++)
    {
        float pointXa=(mySetPoint[lines[j].a].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
        float pointYa=(mySetPoint[lines[j].a].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
        QPoint toLineA(pointXa,pointYa);
        float pointXb=(mySetPoint[lines[j].b].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
        float pointYb=(mySetPoint[lines[j].b].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
        QPoint toLineB(pointXb,pointYb);
        if(lines[j].routecondi==1) //路况正常
        {
            Linepen.setWidth(3); //线条粗细
            Linepen.setColor(Qt::green); //绿色的
            painter.setPen(Linepen);
            if(lines[j].minline==true)
            {
                Linepen.setWidth(5);
                Linepen.setColor(Qt::blue); //选中：蓝色的
                painter.setPen(Linepen);
                painter.drawLine(toLineA,toLineB);
            }
            Linepen.setColor(Qt::green);
            Linepen.setWidth(3);
        }
        else if(lines[j].routecondi==2)
        {
            Linepen.setWidth(3); //线条粗细
            Linepen.setColor(Qt::yellow); //黄色的
            painter.setPen(Linepen);
            if(lines[j].minline==true)
            {
                Linepen.setWidth(5);
                Linepen.setColor(Qt::darkYellow); //选中：深黄的
                painter.setPen(Linepen);
                painter.drawLine(toLineA,toLineB);
            }
            Linepen.setColor(Qt::yellow);
            Linepen.setWidth(3);
        }
        else if(lines[j].routecondi==3)
        {
            Linepen.setWidth(3); //线条粗细
            Linepen.setColor(Qt::red); //红色的
            painter.setPen(Linepen);
            if(lines[j].minline==true)
            {
                Linepen.setWidth(5);
                Linepen.setColor(Qt::darkRed); //选中：深红的
                painter.setPen(Linepen);
                painter.drawLine(toLineA,toLineB);
            }
            Linepen.setColor(Qt::red);
            Linepen.setWidth(3);
        }
    }


    //重要步骤：画点（应在画线操作之后）
    painter.setPen(Qt::black); //设置边框颜色为黑色：预备画点
    painter.setBrush(Qt::yellow); //设置内部填充颜色：预备画点
    painter.setFont(font1); //设置用到的字体：预设一类
    for(int j=0;j<pointNum;j++) //遍历所有点，一个一个画出来。注意这里是j不是i！i现在是缩放系数！乱改会程序错误
    {
        //搭配按钮操作1加点操作已实现
        //先要再度坐标变换适配当前界面
        //painter.drawEllipse(mySetPoint[j].P,4,4);
        float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
        float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
        QPoint toDraw(pointX,pointY); //重新定义当前这个点以适配目前的图片大小，目标是为了画图，所以只用局部变量

        if(mySetPoint[j].hidden==false) //新增玩法：隐藏点，隐藏不画
        {
            if(mySetPoint[j].selected==false)
            {
                painter.drawEllipse(toDraw,3,3); //画出当前这个点，大小为“3”
            }
            if(mySetPoint[j].selected==true)
            {
                painter.setBrush(Qt::red); //变色画法
                font2.setBold(true); //加粗
                painter.setFont(font2); //字体改大一点
                painter.setPen(mycolor);
                painter.drawEllipse(toDraw,3,3);
                painter.setBrush(Qt::yellow); //恢复如初！

                if(butcas==1)
                {
                    mySetPoint[j].selected=false; //仅针对添加点，画完就取消选中
                }
            }

            painter.drawText(QPoint(toDraw.x()+4,toDraw.y()+4),mySetPoint[j].PS); //写下这个点的点名称
            painter.setFont(font1); //恢复如初！
            painter.setPen(Qt::black);
            font2.setBold(false);
        }
    }

    //测试栏
    int visiblePoint=0;
    for(int j=0;j<pointNum;j++)
    {
        if(mySetPoint[j].hidden==false)
        {
            visiblePoint++;
        }
    }
    ui->label_5->setText("当前共有："+QString::number(lineNum)+"条线，"+QString::number(pointNum)+"个点，其中可见点"+QString::number(visiblePoint)+"个。");
}



int selectPoint; //选到点的号码，全局变量方便使用

void Mapedit::mousePressEvent(QMouseEvent* event)
{
    //directed by STAssn
    //关键步骤：鼠标点击触发事件，地图编辑部分重点配置
    //重要前置操作与后期关键设置，鼠标在程序使用中起决定性作用，须好生担待
    //思想：按键控制符号位进而控制鼠标进入情况，针对不同情况做出不同回应
    if (event->button() == Qt::LeftButton) //鼠标左击进入事件，右击无效
    {
        mouseMov[1-miceNum]=event->pos();
        mouseMov[miceNum]=event->pos(); //类成员，用于位置拖移
        QPoint now=event->pos(); //新建一个点，是当前鼠标点到的位置
        float nowX=(now.x()-(650+hrimov))*i/4+650; //坐标变换：先恢复原图大小再转换到标准型，现在得到的是在标准型上的坐标
        float nowY=(now.y()-(300+colmov))*i/4+300; //坐标变换：先恢复原图大小再转换到标准型，现在得到的是在标准型上的坐标
        //ui->label_5->setText(QString::number(300-piccol/2));
        switch (butcas)
        { //分类讨论当前按钮情况
        case 1: //情况1：添加一个点
            if((now.x()>=(650-pichri/2))&&(now.x()<=(650+pichri/2))&&(now.y()>=(300-piccol/2))&&(now.y()<=(300+piccol/2))) //判断点坐标是不是在图形里
            {
                if(pointNum<90)
                {
                    //待解决：点间距可能太近的问题
                    mySetPoint[pointNum].P = event->pos(); //当前点赋值于目前的点结构体的点位
                    mySetPoint[pointNum].PS = QString::number(pointLabel); //给点一个标号，然后全部进一位来到下一个点（类似堆栈的进栈）
                    mySetPoint[pointNum].PN = pointLabel; //点编号
                    //存在坐标变换！如果在图片缩放或移动的情况下选取了这个点，要得到变换坐标
                    //已知固定的中心点确定为(650,350)，左右移动参数分别产生hrimov和colmov的图片中心点移动，缩放变换就是原坐标减去中心点坐标进行变换再加上中心点坐标
                    mySetPoint[pointNum].Px=nowX; //坐标变换：代入
                    mySetPoint[pointNum].Py=nowY; //坐标变换：代入
                    //ui->label_5->setText(QString::number(mySetPoint[pointNum].Px)); //调试使用语句
                    mySetPoint[pointNum].selected=true; //选中了

                    pointNum++; //跳转到下一个点
                    pointLabel++;
                    ui->label_4->setText("已选中点位，可继续选择");

                    update(); //回调：更新图片

                    //butcas=0; //单次加点or连续加点？注释掉以后现在是连续加点
                }
                else
                {
                    QMessageBox::warning(this, "警告", "点数超出范围：最多90个点。"); //提示栏
                }
            }
            else
            {
                ui->label_4->setText("选点超出范围，请重新选择");
                QMessageBox::warning(this, "警告", "选点超出范围，请重新选择"); //提示栏
            }
            break; //简单测试，已基本实现
        case 2: //情况2，取消已有点
            //directed by STAssn
            //有点的线别删，这里好像还有点问题。更新：经过修改简单情况暂时疑似已解决，复杂情况有待观察
            //难度较大，有嵌套重合部分，维护性较低，建议存档以便修改后万一崩溃了可以读档
            //这一步时间复杂度较大，运行效率较低
            if (pointNum == 0)
            {
                QMessageBox::warning(this, "警告", "当前地图上没有点！"); //提示栏：如果没有点
                butcas = 0;
            }
            else
            {
                for(int k=0; k<pointNum; k++)
                {
                    //ui->label_5->setText(QString::number(butcas)); //测试栏
                    float pointX=(mySetPoint[k].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                    float pointY=(mySetPoint[k].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                    QPoint toCheck(pointX,pointY); //比照点
                    if(isin(now,toCheck)) //寻找对应的点，和画的点比照不然无法成功！
                    {
                        int sele; //准备清理

                        //新：改线结构体
                        for(int j=0;j<lineNum;j++)
                        {
                            if((lines[j].a==k)||(lines[j].b==k)) //有这个点的连线
                            {
                                for(int mov=j;mov<lineNum;mov++) //这个点往后全部前移，这个线不要了
                                {
                                    //lines[mov]=lines[mov+1]; //这样做是可以的吗？应该可以，看起来重载过了
                                    lines[mov].a=lines[mov+1].a;
                                    lines[mov].b=lines[mov+1].b;
                                    lines[mov].minline=lines[mov+1].minline;
                                    lines[mov].routecondi=lines[mov+1].routecondi;
                                    //lines[mov].routetype=lines[mov+1].routetype; //这个没了，别在意
                                }
                                lineNum--;
                                j--; //删过线了，现在的j位置其实是没有检查过的，所以减一修正
                            }
                        }
                        for(int j=0;j<lineNum;j++) //因为删点，点编号变过了，线结构体要同步变号
                        {
                            if(lines[j].a>k)
                            {
                                lines[j].a--;
                            }
                            if(lines[j].b>k)
                            {
                                lines[j].b--;
                            }
                        }



                        //改点结构体了
                        for(sele=k;sele<pointNum;sele++)
                        {
                            mySetPoint[sele].P=mySetPoint[sele+1].P; //形式点前移
                            mySetPoint[sele].PS=mySetPoint[sele+1].PS; //点名称前移，不跟随点编号是因为预留开发重命名功能
                            mySetPoint[sele].Px=mySetPoint[sele+1].Px; //点实际横坐标前移
                            mySetPoint[sele].Py=mySetPoint[sele+1].Py; //点实际纵坐标前移
                            //mySetPoint[sele].neighborPoint=mySetPoint[sele+1].neighborPoint; //邻接表指针前移
                            mySetPoint[sele].selected=mySetPoint[sele+1].selected; //是否选中也前移
                            mySetPoint[sele].PN=sele; //点编号变成当前点编号
                            mySetPoint[sele].hidden=mySetPoint[sele+1].hidden;
                            //全部前移

                            //邻接矩阵的修改
                            for(int matr=0;matr<pointNum;matr++)
                            {
                                //邻接矩阵所有位置前移
                                adjacencyMatrix[sele][matr]=adjacencyMatrix[sele+1][matr];
                                adjacencyMatrix[matr][sele]=adjacencyMatrix[matr][sele+1];
                            }

                        }
                        pointLabel=pointLabel-1; //全部减完，点数减一
                        pointNum=pointNum-1; //全部减完，点数减一

                        //QMessageBox::information(this,"提示","还有"+QString::number(pointNum)+"个点");

                        update();
                        break;
                    }
                }
            }
            break; //简单测试，已部分实现，可成功删无连线和有连线的点，复杂情况有待观察暂不确定
        case 3: //情况3，连线·第一点，与情况4搭配使用
            ui->label_4->setText("当前状态：地图上为两点增加连线，请选择第一个点");
            toClear();
            if(lineNum<180)
            {
                for(int j=0;j<pointNum;j++)
                {
                    float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                    float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                    QPoint toCheck(pointX,pointY); //比照点
                    if(isin(now,toCheck))
                    {
                        //匹配到了！
                        ui->label_4->setText("当前状态：地图上为两点增加连线，已选择第一个点");
                        selectPoint=j; //保存一下是哪个点，准备后续操作！
                        butcas =4; //转移到情况4，换第二个点，组成连线
                        mySetPoint[j].selected=true; //选中它
                        update(); //画上去
                    }
                }
            }
            else
            {
                QMessageBox::warning(this,"警告","线路数量已达上限：最多180条。");
            }
            break;
        case 4: //情况4，连线·第二点，与情况3搭配使用
            for(int j=0;j<pointNum;j++)
            {
                //新空间不要用malloc，要用new，可能是版本不适配！
                ui->label_4->setText("当前状态：地图上为两点增加连线，请选择第二个点");
                float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                QPoint toCheck(pointX,pointY); //比照点
                if((isin(now,toCheck))&&(j!=selectPoint)) //不能和之前那点相同哦
                {
                    ui->label_4->setText("当前状态：地图上为两点增加连线，已选择第二个点，可继续操作");
                    mySetPoint[j].selected=true;

                    //邻接矩阵算法
                    adjacencyMatrix[selectPoint][j]=distInPro(selectPoint,j); //两距离直接存入邻接矩阵
                    adjacencyMatrix[j][selectPoint]=distInPro(selectPoint,j);


                    //准备画线，用线结构体
                    int tema,temb;
                    if(selectPoint>j) //先小再大，有序准备，存点编号，这个值设定与点编号相同
                    {
                        tema=j;
                        temb=selectPoint;
                    }
                    else
                    {
                        temb=j;
                        tema=selectPoint;
                    }
                    for(int k=0;k<lineNum;k++)
                    {
                        if((lines[k].a==tema)&&(lines[k].b==temb))
                        {
                            QMessageBox::warning(this, "警告", "路线重复！");
                            mySetPoint[j].selected=false;
                            mySetPoint[selectPoint].selected=false;
                            update();
                            butcas=3;
                            break;
                        }
                    }
                    lines[lineNum].a=tema;
                    lines[lineNum].b=temb;
                    lineNum++; //存入线路
                    update();
                    butcas=3; //回到情况3重复操作
                }
            }
            break;
        case 5: //情况5，取消连线·第一点，与情况6搭配使用
            if (pointNum == 0)
            {
                QMessageBox::warning(this, "警告", "当前地图上没有点！"); //提示栏：如果没有点
                butcas = 0;
            }
            toClear();
            for(int j=0;j<pointNum;j++)
            {
                float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                QPoint toCheck(pointX,pointY); //比照点
                if(isin(now,toCheck))
                {
                    //匹配到了！看看有没有线？
                    //bool checkPoint=false;
                    for(int k=0;k<pointNum;k++)
                    {
                        if(adjacencyMatrix[j][k]!=0)
                        {
                            ui->label_4->setText("当前状态：地图上为两点取消连线，已选择第一个点");
                            selectPoint=j; //保存一下是哪个点，准备后续操作！
                            butcas =6; //转移到情况6，换第二个点，组成连线
                            mySetPoint[j].selected=true; //选中它
                            update(); //画上去
                            break; //有线就结束，不管几条
                        }
                    }
                }
            }
            break;
        case 6: //情况6，取消连线·第二点，与情况5搭配使用
            for(int j=0;j<pointNum;j++)
            {
                float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                QPoint toCheck(pointX,pointY); //比照点
                if(isin(now,toCheck)&&(j!=selectPoint)) //选中点
                {
                    int tema,temb;
                    if(selectPoint>j) //先小再大，有序准备，存点编号，这个值设定与点编号相同
                    {
                        tema=j;
                        temb=selectPoint;
                    }
                    else
                    {
                        temb=j;
                        tema=selectPoint;
                    }
                    for(int k=0;k<lineNum;k++)
                    {
                        if((lines[k].a==tema)&&(lines[k].b==temb)) //选到连线了！
                        {
                            for(int l=k;l<lineNum;l++)
                            { //全！部！前！移！别忘了邻接矩阵
                                //lines[l]=lines[l+1]; //可以这样吗？
                                lines[l].a=lines[l+1].a;
                                lines[l].b=lines[l+1].b;
                                lines[l].routecondi=lines[l+1].routecondi;
                                lines[l].minline=lines[l+1].minline;
                                //lines[l].routetype=lines[l+1].routetype; //这个没了，别在意
                            }
                        }
                    }
                    //邻接矩阵删连接！
                    for(int k=0;k<pointNum;k++)
                    {
                        for(int l=0;l<pointNum;l++)
                        {
                            if(((k==tema)&&(l==temb))||((l==tema)&&(k==temb)))
                            {
                                adjacencyMatrix[k][l]=0; //相应位置置零
                            }
                        }
                    }
                    lineNum--;
                    update();
                    butcas=5;
                }
            }
            break;
        case 7: //情况7：选点求最短路径
            ui->label_4->setText("当前状态：求取耗时最少的路径，请选择第一个点");
            for(int j=0;j<pointNum;j++)
            {
                if(mySetPoint[j].hidden==false)
                {
                    float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                    float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                    QPoint toCheck(pointX,pointY); //比照点
                    if(isin(now,toCheck))
                    {
                        //匹配到了！
                        ui->label_4->setText("当前状态：求取耗时最少的路径，请选择第二个点");
                        selectPoint=j; //保存一下是哪个点，准备后续操作！
                        butcas =8; //转移到情况8，换第二个点，求“最短距离”
                        mySetPoint[j].selected=true; //选中它
                        update(); //画上去
                    }
                }
            }
            break;
        case 8:
            ui->label_4->setText("当前状态：求取耗时最少的路径，请选择第二个点");
            for(int j=0;j<pointNum;j++)
            {
                if(mySetPoint[j].hidden==false)
                {
                    float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                    float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                    QPoint toCheck(pointX,pointY); //比照点
                    if(isin(now,toCheck))
                    {
                        //匹配到了！
                        ui->label_4->setText("当前状态：求取耗时最少的路径，请选择第二个点");
                        mySetPoint[j].selected=true; //选中它
                        //QMessageBox::information(this,"提示","两点已选好，分别是点"+mySetPoint[j].PS+"和"+mySetPoint[selectPoint].PS);
                        shortestRoute(selectPoint,j); //求最短距离
                        update(); //画上去
                        butcas =0;
                        ui->label_4->setText("当前无状态");
                        //
                    }
                }
            }
            break;
        case 9:
            ui->label_4->setText("当前状态：修改路况，请选择第一个点");
            for(int j=0;j<pointNum;j++)
            {
                // if(mySetPoint[j].hidden==false)
                // {
                float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                QPoint toCheck(pointX,pointY); //比照点
                if(isin(now,toCheck))
                {
                    //匹配到了！
                    ui->label_4->setText("当前状态：修改路况，请选择第二个点");
                    selectPoint=j; //保存一下是哪个点，准备后续操作！
                    butcas =10; //转移到情况8，换第二个点，求“最短距离”
                    mySetPoint[j].selected=true; //选中它
                    update(); //画上去
                }
                // }
            }
            break;
        case 10:
            ui->label_4->setText("当前状态：修改路况，请选择第二个点");
            for(int j=0;j<pointNum;j++)
            {
                // if(mySetPoint[j].hidden==false)
                // {
                float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                QPoint toCheck(pointX,pointY); //比照点
                if(isin(now,toCheck))
                {
                    //匹配到了！
                    ui->label_4->setText("当前状态：修改路况，请选择第二个点");
                    mySetPoint[j].selected=true; //选中它
                    int tema,temb;
                    if(selectPoint>j) //先小再大，有序准备，存点编号，这个值设定与点编号相同
                    {
                        tema=j;
                        temb=selectPoint;
                    }
                    else
                    {
                        temb=j;
                        tema=selectPoint;
                    }
                    for(int k=0;k<lineNum;k++)
                    {
                        if((lines[k].a==tema)&&(lines[k].b==temb)) //选到连线了！
                        {
                            // SelectionDialog routecon;
                            // int con=routecon.getSelectedValue();
                            // lines[k].routecondi=con;
                            selectDialog dio;
                            //dio.show();
                            if(dio.exec()==QDialog::Accepted)
                            {
                                lines[k].routecondi=dio.getReturner(); //获取路况修改信息
                            }
                            //QMessageBox::information(this,"提示",QString::number(lines[k].routecondi));
                        }
                    }

                    update(); //画上去
                    butcas =9;
                }
            }
            break;
        case 11: //情况11，点的重命名（给个更好的名字而不是只有一个编号）
            ui->label_4->setText("当前状态：点的重命名");
            for(int j=0;j<pointNum;j++)
            {
                if(mySetPoint[j].hidden==false)
                {
                    float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                    float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                    QPoint toCheck(pointX,pointY); //比照点
                    if(isin(now,toCheck))
                    {
                        //匹配到了！
                        ui->label_4->setText("当前状态：点的重命名");

                        QString pointName = QInputDialog::getText(this, "编辑标签", "输入文本(不超过15个字)");
                        if(pointName!="")
                        {
                            mySetPoint[j].PS=pointName; //重命名之
                        }

                        mySetPoint[j].selected=true; //选中它
                        update(); //画上去
                        butcas=0;
                    }
                }
            }
            break;
        case 12:
            ui->label_4->setText("当前状态：点的隐藏");
            for(int j=0;j<pointNum;j++)
            {
                float pointX=(mySetPoint[j].Px-650)*4/i+(650+hrimov); //从结构体中提取出变换坐标后的坐标并变换之
                float pointY=(mySetPoint[j].Py-300)*4/i+(300+colmov); //从结构体中提取出变换坐标后的坐标并变换之
                QPoint toCheck(pointX,pointY); //比照点
                if(isin(now,toCheck))
                {
                    //匹配到了！
                    ui->label_4->setText("当前状态：点的隐藏");
                    mySetPoint[j].hidden=true; //隐藏了

                    update(); //画上去
                    //butcas=0;
                }
            }
            break;
        }
    }
    if(event->button()==Qt::RightButton) //右击效果
    {
        mouseMov[1-miceNum]=event->pos();
        mouseMov[miceNum]=event->pos(); //类成员，用于位置拖移
        //this->close();
    }

}

//鼠标操作的尝试
void Mapedit::wheelEvent(QWheelEvent *event) //试图用鼠标滑轮控制图片缩放，不能放太大，和滑块不互通
{
    QPointF pos=event->position();
    int x=pos.x();
    int y=pos.y();
    int change=0;
    if((x>=(650-pichri/2))&&(x<=(650+pichri/2))&&(y>=(300-piccol/2))&&(y<=(300+piccol/2))) //判断点坐标是不是在图形里
    {
        QPoint cha=event->angleDelta();
        change=cha.y();
    }
    if(change>0)
    {
        if(i>0.5)
        {
            i=i-0.5;
        }
        else if((i<=0.5)&&(i>0.3))
        {
            i=i-0.1;
        }
        else if(i<=0.3)
        {
            QMessageBox::warning(this,"警告","已经放到最大！");
        }
        //分类计算，不能到0
    }
    else if(change<0)
    {
        if((i>=0.5)&&(i<4))
        {
            i=i+0.5;
        }
        else if((i<0.5)&&(i>0))
        {
            i=i+0.1;
        }
        else if(i>=4)
        {
            QMessageBox::warning(this,"警告","已经缩到最小！");
        }
        //分类计算，不能到0
    }
    update();
}

void Mapedit::mouseMoveEvent(QMouseEvent *event) //鼠标按下移动时的操作，平移图片，与滑块不互通
{
    QPoint posi=event->pos();
    miceNum=1-miceNum;
    mouseMov[miceNum]=event->pos();
    float movX,movY;
    //ui->label_4->setText(QDir::currentPath());
    //ui->label_6->setText(QString::number(mouseMov[miceNum].x()-mouseMov[1-miceNum].x())+" "+QString::number(mouseMov[miceNum].y()-mouseMov[1-miceNum].y())+" "+QString::number(posi.x())+" "+QString::number(posi.y()));
    if((posi.x()>=(650-pichri/2))&&(posi.x()<=(650+pichri/2))&&(posi.y()>=(300-piccol/2))&&(posi.y()<=(300+piccol/2))) //判断点坐标是不是在图形里
    {
        //painter.drawPixmap(650+hrimov-picsize.width()/2,300+colmov-picsize.height()/2,picsize.width(),picsize.height(),pic);
        //在范围里
        QCursor cur;
        cur.setShape(Qt::OpenHandCursor);
        hri=false;
        col=false;
        movX=(mouseMov[miceNum].x()-mouseMov[1-miceNum].x());
        movY=(mouseMov[miceNum].y()-mouseMov[1-miceNum].y());
        //避免放空
        hrimov+=movX; //这个本质是图片中心的水平垂直偏移量
        colmov+=movY;
        //检测并强制控制图片边界的操作放入了绘图的函数里

    }
    update(); //画一画
}

void Mapedit::mouseReleaseEvent(QMouseEvent *event) //鼠标按键释放时效果，坐标对齐，避免新按下时坐标突变
{
    mouseMov[1-miceNum]=mouseMov[miceNum];
}



void Mapedit::on_pushButton_clicked() //点击退出界面
{
    this->close(); //点击按钮退出界面，没啥用做着玩的
}


void Mapedit::on_pushButton_2_clicked() //换张图片显示，图片自选默认路径可更改
{
    QString temp = QFileDialog::getOpenFileName(this, "选择文件", "D:/TEST/QtTest/QTest/", "图像文件(*.jpg *.jpeg *.bmg *.png *.gif)");
    if (temp != "")
    {
        piclo = temp;//传入图片存储路径
    }
    update();
}



void Mapedit::on_horizontalSlider_valueChanged(int value) //滑块槽函数：移动滑块触发：缩放图片
{
    i=4*(1-value*1.00/100); //调整缩放比例，i从0到4可以放大图片
    update(); //回调
    butcas=0; //移动滑块取消当前按钮操作
    ui->label_4->setText("当前无状态");
}


void Mapedit::on_horizontalSlider_2_valueChanged(int value) //滑块槽函数：移动滑块触发：水平移动图片
{
    //水平移动，目标是可以移动但不会移出边框，即边框始终处于填满状态
    //线性变换：[0,99]修改到[-(当前图片宽度/2-标准型图片宽度/2),(当前图片宽度/2-标准型图片宽度/2)]
    //如果极值只到当前图片宽度的一半，最终状态则边框内会有一半是空的，所以再减去标准图片宽度即边框宽度的一半
    //图片最小即全部图片填满边框时，水平调节无效
    //bug:图片最小时移动了滑块，再放大图片时不会按照水平移动情况调节，再次移动滑块有突变式修复，最终效果成立
    valuehri=value; //新：转移实时数据，计算操作转移至回调函数内
    hri=true;
    //hrimov=(value*1.00/99*2*(pichriused/2-pichri/2)-(pichriused/2-pichri/2))*(-1);
    //转换到水平移动尺寸，可以水平移动图片，因为中间有涉及i的部分，已转移至回调函数
    update(); //回调
    butcas=0; //移动滑块取消当前按钮操作
    ui->label_4->setText("当前无状态");
    //hri=false;
}


void Mapedit::on_verticalSlider_valueChanged(int value) //滑块槽函数：移动滑块触发：竖直移动图片
{
    //竖直移动，目标是可以移动但不会移出边框，即边框始终处于填满状态
    //线性变换：[0,99]修改到[-(当前图片高度/2-标准型图片高度/2),(当前图片高度/2-标准型图片高度/2)]
    //如果极值只到当前图片高度的一半，最终状态则边框内会有一半是空的，所以再减去标准图片高度即边框高度的一半
    //图片最小即全部图片填满边框时，竖直调节无效
    //bug:图片最小时移动了滑块，再放大图片时不会按照竖直移动情况调节，再次移动滑块有突变式修复，最终效果成立（已解决）
    valuecol=value;//新：转移实时数据，计算操作转移至回调函数内
    col=true;
    //colmov=value*1.00/99*2*(piccolused/2-piccol/2)-(piccolused/2-piccol/2);
    //转换到竖直移动尺寸，可以竖直移动图片，因为中间有涉及i的部分，已转移至回调函数
    update(); //回调
    butcas=0; //移动滑块取消当前按钮操作
    ui->label_4->setText("当前无状态");
    //col=false;
}


void Mapedit::on_pushButton_3_clicked() //槽函数，地图上选点添加，前期关键步骤
{
    butcas = 1; //规定操作类型
    ui->label_4->setText("当前状态：地图上选择一个点进行添加"); //文字框(标签类)显示文字
    toClear();
}


void Mapedit::on_pushButton_4_clicked() //槽函数，地图上取消已有的点
{
    butcas = 2; //规定操作类型
    ui->label_4->setText("当前状态：取消地图上已经添加的点");
    toClear();
}


void Mapedit::on_pushButton_5_clicked() //槽函数，地图上添加连线，前期关键步骤
{
    butcas = 3; //规定操作类型
    ui->label_4->setText("当前状态：地图上为两点增加连线，请选择第一个点");
    toClear();
}


void Mapedit::on_pushButton_6_clicked() //二级步骤：槽函数，保存线路
{
    //checked by STAssn
    //转换成标准字符后保存以待后续使用
    //字符串依次存储：线路数、点数、点坐标、邻接矩阵、线结构体、线路类型（先这样不够再加）
    using namespace std;
    QString filename = QFileDialog::getSaveFileName(this, "保存文件", "", "地图文件(*.map2)");//文件扩展名为.map2
    if (filename != "")//判断地址是否输入成功
    {
        ofstream out(filename.toStdString());//转换为标准字符串进行保存
        if (out.is_open())
        {
            out << lineNum << ' ' << pointNum << endl; //1,2
            for (int j = 0; j < pointNum; j++)
            {
                out << mySetPoint[j].Px << ' ' << mySetPoint[j].Py << ' ' << mySetPoint[j].PS.toStdString() << ' ' <<mySetPoint[j].hidden << endl;//空格隔开
            } //3,4,5
            for (int k = 0; k < pointNum; k++)
            {
                for (int j = 0; j < pointNum; j++)
                {
                    out << adjacencyMatrix[k][j] << ' '; //邻接矩阵保存
                }
                out << endl;
            } //6
            for (int k = 0; k < lineNum; k++)
            {
                out << lines[k].a << ' ' << lines[k].b << ' ' << lines[k].routecondi << endl; //线结构体保存
            } //7,8,9
            out << piclo.toStdString() << endl; //10
            QMessageBox::information(this, "提示", "保存成功");
            out.close();//关闭文档
        }
        else QMessageBox::information(this, "提示", "保存失败");
    }
}


void Mapedit::on_pushButton_7_clicked() //二级步骤：槽函数，导入线路
{
    //checked by STAssn
    //字符串依次存储：线路数、点数、点坐标、邻接矩阵、线结构体、线路类型
    //一个一个出来
    QString filename = QFileDialog::getOpenFileName(this, "选择文件", "", "地图文件(*.map2)");
    openmap(filename); //函数已经在前面单独实现啦，这里代入就好
}


void Mapedit::on_pushButton_8_clicked() //查询地图状态，用于查看情况
{
    QMessageBox::information(this, "提示", "地图目前有："+QString::number(pointNum)+"个点，\n"+QString::number(lineNum)+"个线路，\n地图是"+piclo+"，\n地图大小："+QString::number(pichri)+"*"+QString::number(piccol));
}


void Mapedit::on_pushButton_9_clicked() //槽函数，地图上取消线
{
    butcas=5; //规定操作类型
    ui->label_4->setText("当前状态：地图上为两点取消连线，请选择第一个点");
    toClear();
}


void Mapedit::on_pushButton_10_clicked() //槽函数，用于切换模式，不进入switch，形同重开
{
    walk=!walk;
    if(walk==true)
    {
        speed=80; //单位分钟，勿忘
        QString filename="../../SHUwalkMap.map2";
        openmap(filename);
        verify(); //拨乱反正
        ui->label_6->setText("当前模式：步行");
    }
    else
    {
        speed=300; //单位分钟
        QString filename="../../SHUrideMap.map2";
        openmap(filename);
        verify(); //拨乱反正
        ui->label_6->setText("当前模式：骑行");
    }
    toClear(); //清理，避免麻烦
    //待续，未完
}


void Mapedit::on_pushButton_11_clicked() //槽函数，求取最短路径
{
    butcas=7;
    ui->label_4->setText("当前状态：求取耗时最少的路径，请选择第一个点");
    toClear();
    //shortestWay=0; //重要！每次算最短路径需要清零距离，否则就会距离一直叠加，放画线函数里了
}


void Mapedit::on_pushButton_12_clicked() //槽函数，附加内容，路况设置
{
    butcas=9;
    ui->label_4->setText("当前状态：修改路况，请选择第一个点");
    toClear();
}


void Mapedit::on_pushButton_13_clicked() //槽函数，点重命名
{
    if (pointNum == 0)
    {
        QMessageBox::warning(this, "警告", "当前图片没有点");
    }
    else
    {
        butcas=11;
        ui->label_4->setText("当前状态：点的重命名");
        toClear();
    }
}


void Mapedit::on_pushButton_15_clicked() //槽函数，取消点的隐藏
{
    for(int j=0;j<pointNum;j++)
    {
        mySetPoint[j].hidden=false;
        update();
    }
}


void Mapedit::on_pushButton_14_clicked() //槽函数，选点隐藏
{
    butcas=12;
    ui->label_4->setText("当前状态：点的隐藏");
    toClear();
}


void Mapedit::on_pushButton_16_clicked() //二期工程：槽函数：一键删点
{
    //三大类成员重器：点、线、邻接矩阵，全部删除
    //“有德者居之”

    //清理所有线
    int sum;
    sum=lineNum;
    for(int j=sum-1;j>=0;j--)
    {
        lines[j].a=0;
        lines[j].b=0;
        lines[j].minline=false;
        lines[j].routecondi=1;
        lineNum--;
    }

    //清理所有点
    sum=pointNum;
    for(int j=sum-1;j>=0;j--)
    {
        mySetPoint[j].PN=0;
        mySetPoint[j].PS="";
        mySetPoint[j].Px=0;
        mySetPoint[j].Py=0;
        mySetPoint[j].hidden=false;
        mySetPoint[j].selected=false;
        pointNum--;
    }

    //清理邻接矩阵
    for(int j=0;j<90;j++)
    {
        for(int k=0;k<90;k++)
        {
            adjacencyMatrix[j][k]=0;
        }
    }
    update();
    pointLabel=1; //序号重置
}


void Mapedit::on_pushButton_17_clicked()
{
    //二期工程：新增：查找点
    toClear();
    ui->label_4->setText("当前状态：查找点");
    QString search;
    bool ser=false;
    //int count=0;
    QString pointName = QInputDialog::getText(this, "查找点", "请输入你要查找的地点的名称（支持模糊查询）：");
    if(pointName!="")
    {
        search=pointName; //暂存之
    }
    for(int j=0;j<pointNum;j++)
    {
        if((mySetPoint[j].PS.contains(search,Qt::CaseInsensitive))&&(mySetPoint[j].hidden==false))
        {
            mySetPoint[j].selected=true; //选中点
            //count++;
            ser=true;
            ui->listWidget->addItem("已找到点："+mySetPoint[j].PS+"，地图坐标是：("+QString::number(mySetPoint[j].Px)+","+QString::number(mySetPoint[j].Py)+")");
            if(mySetPoint[j].PS==search)
            {
                ui->textEdit->setText("精确找到点："+mySetPoint[j].PS+"，地图坐标是：("+QString::number(mySetPoint[j].Px)+","+QString::number(mySetPoint[j].Py)+")");
                ui->textEdit_2->setText("精确找到点："+mySetPoint[j].PS+"，地图坐标是：("+QString::number(mySetPoint[j].Px)+","+QString::number(mySetPoint[j].Py)+")");
            }
        }
    }
    if(ser==false)
    {
        QMessageBox::warning(this, "警告", "没有找到任何点！");
    }
    update(); //画上去
}


void Mapedit::on_pushButton_18_clicked()
{
    //二期工程：新增：修改比例尺
    toClear();
    bool ok; // 用于检查转换是否成功
    QString newpro = QInputDialog::getText(this, "修改比例尺", "比例尺计算方法：实际距离（米） = 坐标距离（标准） * 比例尺。\n请输入修改后的比例尺（当前比例尺为："+QString::number(proportion)+")：", QLineEdit::Normal, "4.13", &ok);


    if (ok && !newpro.isEmpty())
    { // 检查用户是否点击了“确定”且输入不为空
        bool conversionOk; // 用于检查转换是否成功
        float value = newpro.toFloat(&conversionOk); // 转换为float

        float oldone=proportion;
        if (conversionOk)
        { // 如果转换成功
            //QMessageBox::information(this, "输入结果", QString("您输入的浮点数是: %1").arg(value));
            proportion=value; //修改比例尺
            QMessageBox::information(this, "修改成功", "比例尺修改成功，新比例尺为："+QString::number(proportion));
            for(int j=0;j<pointNum;j++)
            {
                for(int k=0;k<pointNum;k++)
                {
                    adjacencyMatrix[j][k]=adjacencyMatrix[j][k]/oldone*proportion; //更新邻接矩阵
                }
            }
        }
        else
        {
            QMessageBox::warning(this, "转换失败", "无法将输入转换为浮点数！无法修改比例尺！");
        }
    }
    else
    {
        QMessageBox::warning(this, "输入错误", "您未输入任何内容或取消了输入。");
    }

}

