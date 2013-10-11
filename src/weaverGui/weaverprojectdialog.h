#ifndef WEAVERPROJECTDIALOG_H
#define WEAVERPROJECTDIALOG_H

#define WPD_CONFIG_FILE     "~/.weaver-gui.conf"

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

namespace Ui {
    class weaverProjectDialog;
}

class weaverProjectDialog : public QDialog {
    Q_OBJECT
public:
    weaverProjectDialog(QWidget *parent = 0);
    ~weaverProjectDialog();

    void getOutImgFile(char *str);
    void getTempDir(char *str);
    void getPiecelistFile(char * str);
    bool getMultiResTiffFlag();
    bool getManualAlignFlag();
    int getMontageNumCols();
    int getMontageNumRows();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::weaverProjectDialog *ui;
    QString outImgFile, tempDir, piecelistFile, fileOpenDir;

private slots:
    void on_startStitchingButton_clicked();
    void on_choosePiecelistFileButton_clicked();
    void on_chooseOutputImageFileButton_clicked();
    void on_chooseTempDirButton_clicked();
    void on_exitButton_clicked();
};

#endif // WEAVERPROJECTDIALOG_H
