#include "weaverprojectdialog.h"
#include "ui_weaverprojectdialog.h"

weaverProjectDialog::weaverProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::weaverProjectDialog)
{
    ui->setupUi(this);
}

weaverProjectDialog::~weaverProjectDialog()
{
    delete ui;
}




void weaverProjectDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void weaverProjectDialog::on_exitButton_clicked()
{
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Question, "Quit", "This will quit the program. Continue ?",
                                          QMessageBox::Ok|QMessageBox::Cancel, this, Qt::Dialog);

    int result = msgBox->exec();
    delete msgBox;

    switch(result)
    {
    case QMessageBox::Cancel:// Do noothing
                    return;
                    break;
    case QMessageBox::Ok: // quit the application
                    exit(0);
                    break;
    }

}

void weaverProjectDialog::on_chooseTempDirButton_clicked()
{
    QFileDialog dirDialog(this, tr("Choose Temporary Directory"), tempDir,
                           tr("Directories (*.*)"));

    dirDialog.setFileMode(QFileDialog::Directory);
    dirDialog.setAcceptMode(QFileDialog::AcceptOpen);
    dirDialog.setViewMode(QFileDialog::Detail);

    dirDialog.show();

    QStringList fileName;
    if (dirDialog.exec())
    {       
        // store the directory last used
        tempDir = dirDialog.selectedFiles().at(0);
    }
    else
        return;

    ui->tmpDirTextEdit->setText(tempDir);
    return;

}

void weaverProjectDialog::on_chooseOutputImageFileButton_clicked()
{
    // open a file browser for icon files.
    QFileDialog fileOpenDialog(this, tr("Choose Output TIFF file"), fileOpenDir,
                           tr("Image Files (*.tif *.tiff *.TIF *.TIFF)"));

    fileOpenDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileOpenDialog.setViewMode(QFileDialog::Detail);
    fileOpenDialog.setDirectory(QDir(fileOpenDir));

    fileOpenDialog.show();

    QStringList fileName;
    if (fileOpenDialog.exec())
    {
        fileName = fileOpenDialog.selectedFiles();
        outImgFile = fileName.at(0);
        // store the directory last used
        fileOpenDir = fileOpenDialog.directory().absolutePath();
    }
    else
        return;

    ui->outTiffFileTextEdit->setText(outImgFile);
    return;
}

void weaverProjectDialog::on_choosePiecelistFileButton_clicked()
{
    // open a file browser for icon files.
    QFileDialog fileOpenDialog(this, tr("Choose Output TIFF file"), fileOpenDir,
                           tr("Piece List File (*.*)"));

    fileOpenDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileOpenDialog.setViewMode(QFileDialog::Detail);
    fileOpenDialog.setDirectory(QDir(fileOpenDir));

    fileOpenDialog.show();

    QStringList fileName;
    if (fileOpenDialog.exec())
    {
        fileName = fileOpenDialog.selectedFiles();
        piecelistFile = fileName.at(0);
        // store the directory last used
        fileOpenDir = fileOpenDialog.directory().absolutePath();
    }
    else
        return;

    ui->piecelistTextEdit->setText(piecelistFile);
    return;
}

void weaverProjectDialog::on_startStitchingButton_clicked()
{
    // Basically here we just ensure the user entered all the info correctly
    QMessageBox *msgBox;

    if(outImgFile.isEmpty())
    {
        msgBox = new QMessageBox(QMessageBox::Information, "Missing field", "Please choose an output Tiff file",
                                 QMessageBox::Ok, this, Qt::Dialog);
        msgBox->exec();
        delete msgBox;
        return;
    }

    if(tempDir.isEmpty())
    {
        msgBox = new QMessageBox(QMessageBox::Information,
                                 "Missing field", "Please choose a temporary work directory",
                                 QMessageBox::Ok, this, Qt::Dialog);
        msgBox->exec();
        delete msgBox;
        return;
    }

    if(piecelistFile.isEmpty())
    {
        msgBox = new QMessageBox(QMessageBox::Information,
                                 "Missing field", "Please choose a piecelist file with a list of all tiles in a left-to-right top-to-bottom order",
                                 QMessageBox::Ok, this, Qt::Dialog);
        msgBox->exec();
        delete msgBox;
        return;
    }

    this->hide();

}

void weaverProjectDialog::getOutImgFile(char *str)
{
    strcpy(str, outImgFile.toLocal8Bit().data());
}

void weaverProjectDialog::getTempDir(char *str)
{
    strcpy(str, tempDir.toLocal8Bit().data());
}

void weaverProjectDialog::getPiecelistFile(char * str)
{
    strcpy(str, piecelistFile.toLocal8Bit().data());
}

bool weaverProjectDialog::getMultiResTiffFlag()
{
    return ui->multiResTiffCheckBox->isChecked();
}

bool weaverProjectDialog::getManualAlignFlag()
{
    return ui->manualAlignCheckBox->isChecked();
}

int weaverProjectDialog::getMontageNumCols()
{
    return ui->montageNumOfColsSpinBox->value();
}

int weaverProjectDialog::getMontageNumRows()
{
    return ui->montageNumOfRowsSpinBox->value();
}
