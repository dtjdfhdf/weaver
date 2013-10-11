#ifndef WEAVERMANUALALIGN_H
#define WEAVERMANUALALIGN_H

#include <QMainWindow>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QPixmap>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QtOpenGL/QGLWidget>
#include "wStitch.h"

#ifdef WIN32
		// Apparently round is missing in VC++
		#define round(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))
#endif

#define WM_ZOOMIN_FACTOR            1.05f
#define WM_ZOOMOUT_FACTOR           0.95f
#define WM_ZOOM_FACTOR_CLAMP        2.0f
#define WM_SCALED_IMAGE_DIM         2048    // the images biggest dimension is scaled down to this to avoid mem alloc errors

#define WM_REF_IMG_FILE                "/home/raj/data/images/10x10_small/10x10.000.tif"
#define WM_APPEND_IMG_FILE             "/home/raj/data/images/10x10_small/10x10.010.tif"

namespace Ui {
    class WeaverManualAlign;
}

class WeaverManualAlign : public QMainWindow {
    Q_OBJECT
public:
    WeaverManualAlign(QWidget *parent);
    ~WeaverManualAlign();

    // overloaded function for loading and displaying images
    void show(char *refImgFile, char *appendImgFile, wStitchLayout layout,
                                 int *appendImgRelXPos, int * appendImgRelYPos);

    Ui::WeaverManualAlign *ui;
protected:
    void changeEvent(QEvent *e);


private:

    QGraphicsView   *gView;
    QGraphicsScene  *imgScene;

    QImage refImg, appendImg;
    QPixmap *refPixmap, *appendPixmap;
    QGraphicsItem *refImgPixmapItem, *appendImgPixmapItem;

    wStitchLayout imgLayout;
    void autoPosition(wStitchLayout layout);
    void resizeEvent ( QResizeEvent * event );

private slots:


private slots:
    void on_WeaverManualAlign_iconSizeChanged(QSize iconSize);
    void on_autoPosPushButton_clicked();
    void on_pushButton_clicked();
    void on_contrastSlider_sliderReleased();
    void on_brightnessSlider_sliderReleased();
    void on_contrastSlider_actionTriggered(int action);
    void on_brightnessSlider_actionTriggered(int action);
    void on_submitPushButton_clicked();
};

class WeaverGraphicsView: public QGraphicsView {

public:
    WeaverGraphicsView(QGraphicsScene *, QWidget *);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // WEAVERMANUALALIGN_H
