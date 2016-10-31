#include "mymouseqlabel.h"


using namespace std;



myMouseQLabel::myMouseQLabel(QWidget *parent):
    QLabel(parent)
{

}

void myMouseQLabel::mouseMoveEvent(QMouseEvent *ev)
{
    if(!occupato && possoDisegnareLinea && disegnoMode)
    {
        if (ev->buttons() == Qt::LeftButton)
        {
            _classeCorrente = cv::GC_FGD;
        }
        else if (ev->buttons() == Qt::RightButton)
        {
            _classeCorrente = cv::GC_BGD;
        }

        int real_x = (int) ev->x() * x_scale;
        int real_y = (int) ev->y() * y_scale;


        cv::circle(grabcut_mask_current,cv::Point(real_x,real_y),brush_size,_classeCorrente,-1);
        if(_classeCorrente == cv::GC_FGD)
            cv::circle(AnteprimaMarcature_current,cv::Point(real_x,real_y),brush_size,cv::Scalar(0,0,255,(double) 1/fattore_colorizzazione),-1);
        else
            cv::circle(AnteprimaMarcature_current,cv::Point(real_x,real_y),brush_size,cv::Scalar(0,255,0,(double) 1/fattore_colorizzazione),-1);
        emit Mouse_Pos();
    }
}

void myMouseQLabel::mousePressEvent(QMouseEvent *ev)
{
    if(!occupato && possoDisegnareLinea && !selettoreIntelligenteMode)
    {
        disegnoMode = 1;
        grabcut_mask_undo = grabcut_mask_current.clone();
        AnteprimaMarcature_undo = AnteprimaMarcature_current.clone();
        emit Mouse_Pressed();
    }
    else if(!occupato && selettoreIntelligenteMode)
    {
        bBox_XStart = (int) ev->x() * x_scale;
        bBox_YStart = (int) ev->y() * y_scale;
        emit Mouse_Pressed();
    }
}

void myMouseQLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if(!occupato && disegnoMode && !selettoreIntelligenteMode)
    {
        disegnoMode = 0;
        emit Mouse_Left();
    }
    else if(!occupato && selettoreIntelligenteMode)
    {
        bBox_XEnd = (int) ev->x() * x_scale;
        bBox_YEnd = (int) ev->y() * y_scale;
        emit Mouse_Left();
    }
}

