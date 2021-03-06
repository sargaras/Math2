#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineSeries>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    E_S=new ModelEMS(0,0.1,0.00001);
    E_M=new Sattelite(0,E_S->T*0.003,0.00001*E_S->T,theta,i,w,a,e,O);

    TVector X0=Sattelite::setInitialPosition(theta,i,w,a,e,O);

    Decorator_moon=new MoonDecorator(0,0.003,0.00001,*E_M,X0);
    Decorator_sun=new SunDecorator(0,0.003,0.00001,*E_M,X0);
    Decorator_all=new SunDecorator(0,0.003,0.00001,*Decorator_moon,X0);

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue((float)E_M->getT0()/E_M->getT1()*100);

    auto thread_1=std::async (std::launch::async, ([&](){
        TIntegrator * integrator=new TDormandPrince();
        integrator->setPrecision(1e-8);
        integrator->Run(E_M);
        delete integrator;
    }));

    auto thread_2=std::async(std::launch::async,([&](){
        TIntegrator * integrator2=new TDormandPrince();
        integrator2->setPrecision(1e-8);
        integrator2->Run(E_S);
        delete integrator2;
    }));

    thread_1.get();



    auto thread_3=std::async(std::launch::async,([&](){
        TIntegrator * integrator3=new TDormandPrince();
        integrator3->setPrecision(1e-8);
        integrator3->Run(Decorator_moon);
        delete integrator3;
    }));


        TIntegrator * integrator4=new TDormandPrince();
        integrator4->setPrecision(1e-8);
        integrator4->Run(Decorator_sun);
        delete integrator4;

    thread_2.get();
    thread_3.get();

    thread_3=std::async(std::launch::async,([&](){
        TIntegrator * integrator3=new TDormandPrince();
        integrator3->setPrecision(1e-8);
        integrator3->Run(Decorator_all);
        delete integrator3;
    }));
thread_3.get();

    setDialog(theta*180/M_PI,i*180/M_PI,w*180/M_PI,a,e,O*180/M_PI);
}


void MainWindow::setDialog(long double theta, long double i, long double w, long double a, long double e, long double O)
{
    std::string string[6];
    string[0]="theta: "+std::to_string(theta);
    string[1]="i: "+std::to_string(i);
    string[2]="w: "+std::to_string(w);
    string[3]="a: "+std::to_string(a);
    string[4]="e: "+std::to_string(e);
    string[5]="O: "+std::to_string(O);

    std::string main_text=string[0]+"\n"+
                          string[1]+"\n"+
            string[2]+"\n"+
            string[3]+"\n"+
            string[4]+"\n"+
            string[5]+"\n";
    ui->textBrowser->setText(QString::fromStdString(main_text));

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setSeries(QLineSeries *series,TMatrix Result,int first,int second,int j,int k,double mas)
{

    for (int i=0;i<Result.GetRowCount();i++)
            {
                if (first==0){
                  *series<<QPointF((Result(i,first)),((Result(i,second+j)-Result(i,second+k))*E_S->a*mas));
                   ui->progressBar->setValue(10+70*(double)i/Result.GetRowCount());
                }
                else if (second==0)
                {
                      *series<<QPointF(((Result(i,first+j)-Result(i,first+k))*E_S->a)*mas,(Result(i,second)));
                        ui->progressBar->setValue(10+70*(double)i/Result.GetRowCount());
                }
                else {
                    long double res1=((Result(i,first+j)-Result(i,first+k))*E_S->a*mas);
                    long double res2=((Result(i,second+j)-Result(i,second+k))*E_S->a*mas);
                    *series<<QPointF(res1,res2);
                    ui->progressBar->setValue(10+70*(double)i/Result.GetRowCount());
                }
            }
}


void MainWindow::on_pushButton_2_clicked()
{
    ui->graphicsView->clearFocus();
    std::future<void> thread;

    ui->progressBar->setValue(0);

    if((ui->checkBox->isChecked())&&(ui->lineEdit->text().isEmpty())) return;


    if((ui->checkBox->isChecked()))
    {

        if(!ui->lineEdit_2->text().isEmpty()){
            if ((ui->lineEdit_2->text().toDouble()>=0)&&(ui->lineEdit_2->text().toDouble()<360))
                theta=ui->lineEdit_2->text().toDouble()*M_PI/180;
        }
        if(!ui->lineEdit_3->text().isEmpty()){
            if ((ui->lineEdit_3->text().toDouble()>=0)&&(ui->lineEdit_3->text().toDouble()<=90))
                i=ui->lineEdit_3->text().toDouble()*M_PI/180;
        }
        if(!ui->lineEdit_4->text().isEmpty()){
            if ((ui->lineEdit_4->text().toDouble()>=-90)&&(ui->lineEdit_4->text().toDouble()<=90))
                w=ui->lineEdit_4->text().toDouble()*M_PI/180;
        }
        if(!ui->lineEdit_5->text().isEmpty()){
            if ((ui->lineEdit_5->text().toDouble()>=6500000)&&(ui->lineEdit_5->text().toDouble()<=50000000))
                a=ui->lineEdit_5->text().toDouble();
        }
        if(!ui->lineEdit_6->text().isEmpty()){
            if ((ui->lineEdit_6->text().toDouble()>=0)&&(ui->lineEdit_6->text().toDouble()<=0.5))
                e=ui->lineEdit_6->text().toDouble();
        }
        if(!ui->lineEdit_7->text().isEmpty()){
            if ((ui->lineEdit_7->text().toDouble()>=0)&&(ui->lineEdit_7->text().toDouble()<360))
                O=ui->lineEdit_7->text().toDouble()*M_PI/180;
        }

        setDialog(theta*180/M_PI,i*180/M_PI,w*180/M_PI,a,e,O*180/M_PI);

        if(ui->comboBox_3->currentText()=="Earth-Sattelite")
        {
            delete E_M;
             E_M=new Sattelite(0,ui->lineEdit->text().toDouble()*E_S->T,0.001*ui->lineEdit->text().toDouble()*E_S->T,theta,i,w,a,e,O);
            thread=std::async (std::launch::async, ([&](){
                TIntegrator * integrator=new TDormandPrince();
                integrator->setPrecision(1e-12);
                integrator->Run(E_M);
                delete integrator;
            }));
        }
        else if((ui->comboBox_3->currentText()=="Earth-Moon")
                ||(ui->comboBox_3->currentText()=="Sun-Earth")
                ||(ui->comboBox_3->currentText()=="Sun-Moon"))
        {
            delete E_S;
            E_S=new ModelEMS(0,ui->lineEdit->text().toDouble(),0.001*ui->lineEdit->text().toDouble());
            thread=std::async(std::launch::async,([&](){
                TIntegrator * integrator2=new TDormandPrince();
                integrator2->setPrecision(1e-12);
                integrator2->Run(E_S);
                delete integrator2;
            }));
        } else if (ui->comboBox_3->currentText()=="Earth-Sattelite(with Moon data)")
        {        
            thread=std::async(std::launch::async,([&](){
                TVector X0=Sattelite::setInitialPosition(theta,i,w,a,e,O);
                Decorator_moon=new MoonDecorator(0,
                                                 ui->lineEdit->text().toDouble(),
                                                 0.001*ui->lineEdit->text().toDouble(),
                                                 *E_M,
                                                 X0);
                TIntegrator * integrator3=new TDormandPrince();
                integrator3->setPrecision(1e-12);

                integrator3->Run(Decorator_moon);
                delete integrator3;
            }));
        } else if (ui->comboBox_3->currentText()=="Earth-Sattelite(with Sun data)")
        {
            thread=std::async(std::launch::async,([&](){
                TVector X0=Sattelite::setInitialPosition(theta,i,w,a,e,O);
                Decorator_sun=new SunDecorator(0,
                                               ui->lineEdit->text().toDouble(),
                                               0.001*ui->lineEdit->text().toDouble(),
                                               *E_M,
                                               X0);
                TIntegrator * integrator3=new TDormandPrince();
                integrator3->setPrecision(1e-12);
                integrator3->Run(Decorator_sun);
                delete integrator3;
            }));
        } else if (ui->comboBox_3->currentText()=="Earth-Sattelite(with Full data)")
        {
            thread=std::async(std::launch::async,([&](){
                TVector X0=Sattelite::setInitialPosition(theta,i,w,a,e,O);
                Decorator_all=new SunDecorator(0,
                                               ui->lineEdit->text().toDouble(),
                                               0.001*ui->lineEdit->text().toDouble(),
                                               *Decorator_moon,
                                               X0);
                TIntegrator * integrator3=new TDormandPrince();
                integrator3->setPrecision(1e-12);
                integrator3->Run(Decorator_all);
                delete integrator3;
            }));
        }
    }


    QLineSeries *series=new QLineSeries();

  //t,x,y,z
    QString arg[]={"t","x","y","z"};
    int first=0,second=0;

    for(int i=0;i<4;i++)
    {
     if (ui->comboBox->currentText()==arg[i])
     {
         first=i;
     }

     if (ui->comboBox_2->currentText()==arg[i])
     {
         second=i;
     }
    }
    ui->progressBar->setValue(10);

    TMatrix Result;


    if(ui->checkBox->isChecked()){
        thread.get();
    }

    ui->progressBar->setValue(50);

    if(ui->comboBox_3->currentText()=="Earth-Sattelite")
    {
        Result=E_M->getResult();
        for (int i=0;i<Result.GetRowCount();i++)
                {
                       *series << QPointF(  (qreal)Result(i,first),(qreal)Result(i,second) );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    }
    else if(ui->comboBox_3->currentText()=="Earth-Moon")
    {
             setSeries(series,E_S->getResult(),first,second,0,3,1);
    } else if (ui->comboBox_3->currentText()=="Sun-Earth"){
        setSeries(series,E_S->getResult(),first,second,3,6,1e-8);
    } else if(ui->comboBox_3->currentText()=="Sun-Moon"){
             setSeries(series,E_S->getResult(),first,second,0,6,1e-8);
    } else if(ui->comboBox_3->currentText()=="Earth-Sattelite(with Moon data)")
    {
        Result=Decorator_moon->getResult();
        for (int i=0;i<Result.GetRowCount();i++)
                {
                       *series << QPointF(  (qreal)Result(i,first),(qreal)Result(i,second) );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    } else if(ui->comboBox_3->currentText()=="Earth-Sattelite(with Sun data)")
    {
        Result=Decorator_sun->getResult();
        for (int i=0;i<Result.GetRowCount();i++)
                {
                       *series << QPointF(  (qreal)Result(i,first),(qreal)Result(i,second) );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    } else if(ui->comboBox_3->currentText()=="Earth-Sattelite(with Full data)")
    {
        Result=Decorator_all->getResult();
        for (int i=0;i<Result.GetRowCount();i++)
                {
                       *series << QPointF(  (qreal)Result(i,first),(qreal)Result(i,second) );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    }

    if((ui->comboBox_3->currentText()=="Mistake(Classic - Moon)")&&(second!=first))
    {
        TMatrix Result1=E_M->getResult();
        TMatrix Result2=Decorator_moon->getResult();
        for (int i=0;i<Result1.GetRowCount();i++)
                {
                        qreal res1,res2;
                        if (first==0)
                        res1=(qreal)(Result1(i,first));
                        else {
                          res1=(qreal)(Result1(i,first)-Result2(i,first));
                        }

                        if (second==0)
                        res2=(qreal)(Result2(i,second));
                        else {
                          res2=(qreal)(Result1(i,second)-Result2(i,second));
                        }

                       *series << QPointF(  res1,res2 );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    }

    if((ui->comboBox_3->currentText()=="Mistake(Classic - Sun)")&&(second!=first))
    {
        TMatrix Result1=E_M->getResult();
        TMatrix Result2=Decorator_sun->getResult();
        for (int i=0;i<Result1.GetRowCount();i++)
                {
            qreal res1,res2;
            if (first==0)
            res1=(qreal)(Result1(i,first));
            else {
              res1=(qreal)(Result1(i,first)-Result2(i,first));
            }

            if (second==0)
            res2=(qreal)(Result2(i,second));
            else {
              res2=(qreal)(Result1(i,second)-Result2(i,second));
            }
                       *series << QPointF(  res1,res2 );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    }

    if((ui->comboBox_3->currentText()=="Mistake(Classic - Full)")&&(second!=first))
    {
        TMatrix Result1=E_M->getResult();
        TMatrix Result2=Decorator_all->getResult();
        for (int i=0;i<Result1.GetRowCount();i++)
                {
            qreal res1,res2;
            if (first==0)
            res1=(qreal)(Result1(i,first));
            else {
              res1=(qreal)(Result1(i,first)-Result2(i,first));
            }

            if (second==0)
            res2=(qreal)(Result1(i,second));
            else {
              res2=(qreal)(Result1(i,second)-Result2(i,second));
            }
                       *series << QPointF(  res1,res2 );
                       ui->progressBar->setValue(50+30*(double)i/Result.GetRowCount());
                }
    }


    ui->progressBar->setValue(80);

    //создаем график и добавляем в него синусоиду
    QChart *chart=new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    if ((ui->comboBox_3->currentText()=="Sun-Earth")
            ||(ui->comboBox_3->currentText()=="Sun-Moon"))
        chart->setTitle("График: масштаб 1e+8");
    else
        chart->setTitle("График");

    //настройка осей графика
    QValueAxis *axisX=new QValueAxis();
    axisX->setTitleText(arg[first]);

    if ((ui->comboBox_3->currentText()!="Mistake(Classic - Moon)")
    && (ui->comboBox_3->currentText()!="Mistake(Classic - Sun)")
            && (ui->comboBox_3->currentText()!="Mistake(Classic - Full)"))
    {
        axisX->setLabelFormat("%i");
    }
    else
    {
        axisX->setLabelFormat("%f");
    }

    chart->addAxis(axisX,Qt::AlignBottom);
    series->attachAxis(axisX);
    //настройка осей графика


    QValueAxis *axisY=new QValueAxis();
    axisY->setTitleText(arg[second]);

    if ((ui->comboBox_3->currentText()!="Mistake(Classic - Moon)")
    && (ui->comboBox_3->currentText()!="Mistake(Classic - Sun)")
            && (ui->comboBox_3->currentText()!="Mistake(Classic - Full)"))
    axisY->setLabelFormat("%i");
    else
    axisY->setLabelFormat("%f");


    chart->addAxis(axisY,Qt::AlignLeft);
    series->attachAxis(axisY);

    ui->graphicsView->setRubberBand(QChartView::RectangleRubberBand);

    //создаем представление графика
    ui->progressBar->setValue(90);

    ui->graphicsView->setChart(chart);

    ui->progressBar->setValue(100);
}

void MainWindow::on_MainWindow_iconSizeChanged(const QSize &iconSize)
{
   // ui->graphicsView->setSizePolicy(&iconSize);
     //   QChart *chart=new QChart();
    ui->graphicsView->setSizeIncrement(iconSize);

}
