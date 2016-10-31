#ifndef MYMOUSEQLABEL_H
#define MYMOUSEQLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QEvent>
#include <iostream>

#include "utilita.h"


class myMouseQLabel : public QLabel
{
    Q_OBJECT

    private:

        bool disegnoMode = 0;

    public:

        explicit myMouseQLabel(QWidget *parent = 0);
        void mouseMoveEvent(QMouseEvent *ev);
        void mousePressEvent(QMouseEvent *ev);
        void mouseReleaseEvent(QMouseEvent *ev);

    signals:

        void Mouse_Pressed();
        void Mouse_Pos();
        void Mouse_Left();

    public slots:

};



#endif // MYMOUSEQLABEL_H
