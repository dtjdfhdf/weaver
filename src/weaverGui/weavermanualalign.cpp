#include "weavermanualalign.h"
#include "ui_weavermanualalign.h"
#include "qImageOperations.h"

bool g_zoomMouseButtonFlag;
float g_viewZoomFactor = 1.0f;
double g_imgScale = 1.0f; // amount by which the original images are scaled within the tool
int g_mouseRefY, g_mouseRefX;
int *g_appendImgRelXPos, *g_appendImgRelYPos;
WeaverManualAlign *g_winPtr;
QGLWidget *glObject;

WeaverManualAlign::WeaverManualAlign(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WeaverManualAlign)
{
    ui->setupUi(this);
    g_winPtr = this;

    // set up the scene object
    imgScene = new QGraphicsScene();
    imgScene->setBackgroundBrush(QBrush(QColor(150,150,75,255))); // set background to a neutral'ish color
    imgScene->setSceneRect(ui->glFbFrame->x(), ui->glFbFrame->y(),
                        ui->glFbFrame->width(), ui->glFbFrame->height());

    // setup the view object that will actually render the scene
    gView = new WeaverGraphicsView(imgScene, ui->glFbFrame);
    glObject = new QGLWidget(QGLFormat(QGL::SingleBuffer | QGL::SampleBuffers), ui->glFbFrame);
    gView->setViewport(glObject);

}

WeaverManualAlign::~WeaverManualAlign()
{
    delete imgScene;
    delete gView;
    delete ui;
    delete glObject;

}

// over laoded function to load images and show them with UI
void WeaverManualAlign::show(char *refImgFile, char *appendImgFile, wStitchLayout layout,
                             int *appendImgRelXPos, int * appendImgRelYPos)
{
    QMessageBox *aboutBox;
    QString tempQStr;

    tempQStr.sprintf("Could not find overlap between tiles: \"%s\" and \"%s\"\nPlease align them manually and submit their positions. ", refImgFile, appendImgFile);
    aboutBox = new QMessageBox(QMessageBox::Information, "Weaver needs attention",
                               tempQStr, QMessageBox::Ok, this,  Qt::Dialog);
    aboutBox->exec();
    delete aboutBox;

    //
    imgLayout = layout;

    g_appendImgRelXPos = appendImgRelXPos;
    g_appendImgRelYPos = appendImgRelYPos;

    *g_appendImgRelXPos = *g_appendImgRelYPos = 0;
    refPixmap = NULL;
    appendPixmap = NULL;

    // initial status messages   
    tempQStr.sprintf("*****   INSTRUCTIONS    ******");
    ui->statusTextBrowser->append(tempQStr);
    tempQStr.sprintf("Use left Mouse button to select and move. Right mouse button to zoom.");
    ui->statusTextBrowser->append(tempQStr);
    tempQStr.sprintf("Press Ctrl and click on both images to lock their positions\n");
    ui->statusTextBrowser->append(tempQStr);

    // set slider positions
    ui->brightnessSlider->setValue(0);
    ui->contrastSlider->setValue(50);

    QImage *tempImg;
    int scaledWidth = -1, scaledHeight = -1;
    // Load the images and then make a pixmap representation of it. We do it
    // this way cause its easier to manipulate pixels as QImage for brightness/contrast
    if((tempImg = new QImage(refImgFile)) == NULL)
    {
        // looks like the image was not loaded
        tempQStr.sprintf("Error loading image file : %s",refImgFile);
        aboutBox = new QMessageBox(QMessageBox::Information, "Error",
                                                tempQStr,
                                                QMessageBox::Ok, this,  Qt::Dialog);
        aboutBox->exec();
        delete aboutBox;
    }
    else
    {
        // calculate the image scale
        if(tempImg->width() >= tempImg->height())
        {
            scaledWidth = WM_SCALED_IMAGE_DIM;
            g_imgScale = (double)tempImg->width() / (double)WM_SCALED_IMAGE_DIM;
            scaledHeight = (int)round((double)tempImg->height() / g_imgScale);
        }
        else
        {
            scaledHeight = WM_SCALED_IMAGE_DIM;
            g_imgScale = (double)tempImg->height() / (double)WM_SCALED_IMAGE_DIM;
            scaledWidth = (int)round((double)tempImg->width() / g_imgScale);
        }

        // scale down the images to something small to keep memory footprint minimal
        refImg = tempImg->scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio);
        delete tempImg;
        refPixmap = new QPixmap();
        *refPixmap = QPixmap::fromImage(refImg);
        tempQStr.sprintf("Loaded image 1 : %s",refImgFile);
        ui->statusTextBrowser->append(tempQStr);
    }

    if((tempImg = new QImage(appendImgFile)) == NULL)
    {
        // looks like the image was not loaded
        tempQStr.sprintf("Error loading image file : %s",appendImgFile);
        aboutBox = new QMessageBox(QMessageBox::Information, "Error",
                                                tempQStr,
                                                QMessageBox::Ok, this,  Qt::Dialog);
        aboutBox->exec();
        delete aboutBox;
    }
    else
    {
        // scale down the images to something small to keep memory footprint minimal
        appendImg = tempImg->scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio);
        delete tempImg;
        appendPixmap = new QPixmap();
        *appendPixmap = QPixmap::fromImage(appendImg);
        tempQStr.sprintf("Loaded image 2 : %s",appendImgFile);
        ui->statusTextBrowser->append(tempQStr);
    }


    // add ref img to the scene
    refImgPixmapItem = imgScene->addPixmap(*refPixmap);
    refImgPixmapItem->setScale(g_imgScale);
    refImgPixmapItem->setActive(true);
    refImgPixmapItem->setFlag(QGraphicsItem::ItemIsMovable);
    refImgPixmapItem->setFlag(QGraphicsItem::ItemIsFocusable);
    refImgPixmapItem->setFlag(QGraphicsItem::ItemIsSelectable);

    // add append Img to the scene
    appendImgPixmapItem = imgScene->addPixmap(*appendPixmap);
    appendImgPixmapItem->setScale(g_imgScale);
    appendImgPixmapItem->setFlag(QGraphicsItem::ItemIsMovable);
    appendImgPixmapItem->setFlag(QGraphicsItem::ItemIsFocusable);
    appendImgPixmapItem->setFlag(QGraphicsItem::ItemIsSelectable);

    gView->show();

    // position the images the best way we can
    autoPosition(imgLayout);

    // guess the initial zoom factor based on the image and viewport size
    float initialZoomFactor = 0.4f * (float)ui->glFbFrame->width() / ((float)refImg.width() * g_imgScale);
    gView->scale(initialZoomFactor, initialZoomFactor);

    // call the actual show() for this class
    ((QWidget *)this)->show();

    // alert user
    QApplication::beep();

}

void WeaverManualAlign::autoPosition(wStitchLayout layout)
{
    if(layout == _WSM_HORIZONTAL)
        appendImgPixmapItem->setPos(refImgPixmapItem->x() + (refImg.width() + 10) * g_imgScale,
                                    refImgPixmapItem->y());
    else
        appendImgPixmapItem->setPos(refImgPixmapItem->x(),
                                    refImgPixmapItem->y() + (refImg.height() + 10) * g_imgScale );

    gView->centerOn(refImgPixmapItem->x() + (refImg.width() * 2) * g_imgScale,
                    refImgPixmapItem->y() + (refImg.height()* 2) * g_imgScale);
}

void WeaverManualAlign::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void WeaverManualAlign::on_submitPushButton_clicked()
{
    if(*g_appendImgRelXPos == 0 && *g_appendImgRelYPos == 0)
    {
        // looks like the images were not really moved
        QMessageBox *aboutBox = new QMessageBox(QMessageBox::Information, "Alert",
                                                "Please adjust the image positions before submitting them",
                                                QMessageBox::Ok, this,  Qt::Dialog);
        aboutBox->exec();

        return;
    }

    // cleanup the object that were created during show()
    imgScene->removeItem(refImgPixmapItem);
    imgScene->removeItem(appendImgPixmapItem);
    if(refPixmap) delete refPixmap;
    if(appendPixmap) delete appendPixmap;

    this->hide();
}





WeaverGraphicsView::WeaverGraphicsView(QGraphicsScene *s, QWidget *parent)
    :QGraphicsView(s, parent)
 {

 }

void WeaverGraphicsView::mousePressEvent(QMouseEvent *event)
{

    switch (event->button())
    {

        case Qt::LeftButton:         

                break;

        case Qt::RightButton:
                g_zoomMouseButtonFlag = true;
                g_mouseRefX = event->x();
                g_mouseRefY = event->y();
                break;
    } // switch(event->button())

    // retransmit event to parent.. I guess (??)
    QGraphicsView::mousePressEvent(event);
    return;

}

void WeaverGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button())
    {

        case Qt::LeftButton:
                break;

        case Qt::RightButton: g_zoomMouseButtonFlag = false;
                break;
    } // switch(event->button())

    // retransmit event to parent..
    QGraphicsView::mouseReleaseEvent(event);
    return;
}

void WeaverGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QString tempQStr;

    if(g_zoomMouseButtonFlag)
    {
        if((event->x() - g_mouseRefX) > 0)
            g_viewZoomFactor = WM_ZOOMIN_FACTOR;
        else
            g_viewZoomFactor = WM_ZOOMOUT_FACTOR;

        // update the canvas' zoom
        scale(g_viewZoomFactor, g_viewZoomFactor);
    }

    // If the mouse was moved, note the movement
    int refImgXPos, refImgYPos, appendImgXPos, appendImgYPos;

    refImgXPos = scene()->items().at(0)->pos().x();
    refImgYPos = scene()->items().at(0)->pos().y();
    appendImgXPos = scene()->items().at(1)->pos().x();
    appendImgYPos = scene()->items().at(1)->pos().y();

    *g_appendImgRelXPos = (int)round((double)(refImgXPos - appendImgXPos)/scene()->items().at(0)->scale() * g_imgScale);
    *g_appendImgRelYPos = (int)round((double)(refImgYPos - appendImgYPos)/scene()->items().at(0)->scale() * g_imgScale);

    // print the values to the widgets
    tempQStr.sprintf("%d", *g_appendImgRelXPos);
    g_winPtr->ui->xPosTextEdit->setText(tempQStr);
    tempQStr.sprintf("%d", *g_appendImgRelYPos);
    g_winPtr->ui->yPosTextEdit->setText(tempQStr);

    // retransmit event to parent..
    QGraphicsView::mouseMoveEvent(event);

}

void WeaverGraphicsView::keyPressEvent(QKeyEvent *event)
{

    switch(event->key())
    {
    case Qt::Key_Minus:
        g_viewZoomFactor = WM_ZOOMOUT_FACTOR;
        // update the canvas' zoom
        this->scale(g_viewZoomFactor, g_viewZoomFactor);
        break;
    case Qt::Key_Plus:
        g_viewZoomFactor = WM_ZOOMIN_FACTOR;
        // update the canvas' zoom
        this->scale(g_viewZoomFactor, g_viewZoomFactor);
        break;
    }


    QGraphicsView::keyPressEvent(event);

}


void WeaverManualAlign::on_brightnessSlider_actionTriggered(int action)
{

}

void WeaverManualAlign::on_contrastSlider_actionTriggered(int action)
{

}

void WeaverManualAlign::on_brightnessSlider_sliderReleased()
{
    QImage tempImg;
    // change the brightness and contrast of the ref Image
    tempImg = changeBrightness(refImg, ui->brightnessSlider->value());
    tempImg = changeContrast(tempImg, ui->contrastSlider->value());
    ((QGraphicsPixmapItem *)refImgPixmapItem)->setPixmap(QPixmap::fromImage(tempImg));

    // change brightness of the append Img
    tempImg = changeBrightness(appendImg, ui->brightnessSlider->value());
    tempImg = changeContrast(tempImg, ui->contrastSlider->value());
    ((QGraphicsPixmapItem *)appendImgPixmapItem)->setPixmap(QPixmap::fromImage(tempImg));

    return;
}

void WeaverManualAlign::on_contrastSlider_sliderReleased()
{
    QImage tempImg;
    // change the brightness of the refimage
    tempImg = changeBrightness(tempImg, ui->brightnessSlider->value());
    tempImg = changeContrast(refImg, ui->contrastSlider->value());
    ((QGraphicsPixmapItem *)refImgPixmapItem)->setPixmap(QPixmap::fromImage(tempImg));

    // change contrast of append Img
    tempImg = changeBrightness(tempImg, ui->brightnessSlider->value());
    tempImg = changeContrast(appendImg, ui->contrastSlider->value());
    ((QGraphicsPixmapItem *)appendImgPixmapItem)->setPixmap(QPixmap::fromImage(tempImg));

    return;
}

void WeaverManualAlign::on_pushButton_clicked()
{
    ((QGraphicsPixmapItem *)refImgPixmapItem)->setPixmap(*refPixmap);
    ((QGraphicsPixmapItem *)appendImgPixmapItem)->setPixmap(*appendPixmap);

    ui->brightnessSlider->setValue(0);
    ui->contrastSlider->setValue(50);

    return;
}

void WeaverManualAlign::on_autoPosPushButton_clicked()
{
    // position the images the best way we can
    autoPosition(imgLayout);
    return;
}



void WeaverManualAlign::on_WeaverManualAlign_iconSizeChanged(QSize iconSize)
{

}

void WeaverManualAlign::resizeEvent ( QResizeEvent * event )
{
    // handle the widget sizes here
    // the glFbFrame
    ui->glFbFrame->resize(event->size().width() - 20, event->size().height() - ui->mainUiGroupBox->height() - 40);
    ui->mainUiGroupBox->move((event->size().width() - ui->mainUiGroupBox->width())/2, event->size().height() - ui->mainUiGroupBox->height() - 20);

    glObject->resize(ui->glFbFrame->size());
    gView->resize(glObject->size());
}
