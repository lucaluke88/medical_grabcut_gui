#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Altre azioni, come il ripristino delle impostazioni
    ripristinaImpostazioni();
    utilita::showAlert(QString::fromStdString("Traccia col tasto sinistro del mouse per tracciare le zone malate\ncol tasto destro per le zone malate."));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// OpenImage menu item
void MainWindow::on_actionOpen_image_triggered()
{
    // importo l'immagine da disco

    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"),QDir::currentPath(),tr("Image Files [ *.jpg , *.jpeg , *.bmp , *.png , *.gif]"));
    char* charFileName = fileName.toLocal8Bit().data();
    ImmagineOriginale = cv::imread(charFileName);

    if(ImmagineOriginale.data) // Se la risorsa esiste
    {
        resetVariabili(); // pulisci l'ambiente di lavoro
        ui->nomeFileLabel->setText(QString::fromStdString(charFileName));
        init(1);
    }
}

void MainWindow::updatePreview(cv::Mat &mat,int where)
{
    if(where == 1)
        ui->OriginalImageLabel->setPixmap(QPixmap::fromImage(QtOcv::mat2Image(mat)));
    else if(where == 2)
        ui->PelleSana_LabelImg->setPixmap(QPixmap::fromImage(QtOcv::mat2Image(mat)));
    else if(where == 3)
        ui->PelleMalata_LabelImg->setPixmap(QPixmap::fromImage(QtOcv::mat2Image(mat)));
}

void MainWindow::on_actionSegna_pelle_malata_triggered()
{
    _classeCorrente = GC_FGD;
    ui->pixelClassLabel->setText("Pelle malata");
    ui->actionDisplayCurrentMarcature->setIcon(QIcon(":/icons/quadratoRosso"));
}

void MainWindow::on_brushSizeSelector_valueChanged(int value)
{
    brush_size = (int) value/2;
    ui->sliderValueLabel->setText(QString::number(value));
}

// Inserisci marcatura
void MainWindow::MouseLeft_Received()
{
    ui->actionAnnulla->setEnabled(true);

    if(selettoreIntelligenteMode)
    {
        // per poter annullare
        grabcut_mask_undo = grabcut_mask_current.clone();
        AnteprimaMarcature_undo = AnteprimaMarcature_current.clone();

        // disegno il rettangolo

        cv::Scalar blueScalar = cv::Scalar( 255, 0, 0 );
        cv::Point pt_start(bBox_XStart,bBox_YStart), pt_end(bBox_XEnd,bBox_YEnd);
        std::cout << pt_start << "," << pt_end << std::endl;
        cv::rectangle(AnteprimaMarcature_current, pt_start, pt_end, blueScalar, 5, 8, 0);
        selettoreIntelligenteMode = 0;
        updatePreview(AnteprimaMarcature_current,1);

        // fare grabcut locale
        cv::Mat selezione_img = ImmagineOriginale( cv::Rect(pt_start,pt_end));
        cv::Mat selezione_mask = grabcut_mask_current( cv::Rect(pt_start,pt_end));

        segnaOccupato(true);
        utilita::localGrabcut(selezione_img,selezione_mask);


        // Aggiornamento anteprima Immagine
        for(int y = bBox_YStart; y < bBox_YEnd; y ++)
        {
            for(int x = bBox_XStart; x < bBox_XEnd; x ++)
            {
                //cv::Vec3b imgPixelVal = utilita::getPixelValue(y,x,ImmagineOriginale);

                if(grabcut_mask_current.at<uchar>(y,x)==GC_BGD || grabcut_mask_current.at<uchar>(y,x)==GC_PR_BGD)
                    utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,255,0);
                else
                   utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,0,255);

            }
        }
        updatePreview(AnteprimaMarcature_current,1);
        segnaOccupato(false);

    }

    if(segmentazione_continua)
        on_actionApplica_segmentazione_triggered();
}

void MainWindow::MousePos_Received()
{
    if(_classeCorrente == cv::GC_BGD)
        on_actionSegna_pelle_sana_triggered();
    else if (_classeCorrente == cv::GC_FGD)
        on_actionSegna_pelle_malata_triggered();

    // disegna il cerchio

    // usare la variabile temporanea point per poi usarla qua

    updatePreview(AnteprimaMarcature_current,1);

}

void MainWindow::on_actionApplica_segmentazione_triggered()
{
    // controlla se abbiamo qualche seed malato

    if(utilita::seedMalatoExists(grabcut_mask_current))
    {
        segnaOccupato(1);
        ui->progressBar_2->setValue(0);
        cv::Mat foreground(ImmagineOriginale.size(),CV_8UC3,cv::Scalar(0,0,0));
        cv::Mat background(ImmagineOriginale.size(),CV_8UC3,cv::Scalar(0,0,0));

        ui->progressBar_2->setValue(10);
        int nIterazioni = ui->nIterazioniSlider->value();

        int percentuale_stato_parziale = (int) 90 / nIterazioni;
        auto t1 = Clock::now();
        for(int cont = 0; cont < nIterazioni; cont++)
        {
            utilita::myGrabCut(ImmagineOriginale, grabcut_mask_current,foreground,background,nIterazioni,1,usaFastGrabcut,fattoreScala);
            int perc = utilita::percentualePelleMalata(grabcut_mask_current);
            ui->areaMalataPercentualeLabel->setText(QString::number(perc)+QString::fromStdString(" %"));

            // prepara le due anteprime della segmentazione
            ui->progressBar_2->setValue(ui->progressBar_2->value()+percentuale_stato_parziale);
            PelleSanaPreview = ImmagineOriginale.clone();
            PelleMalataPreview = ImmagineOriginale.clone();

            for(int y = 0; y < image_heigth; y ++)
            {
                for(int x = 0; x < image_width; x ++)
                {
                    uchar val = grabcut_mask_current.at<uchar>(y,x);
                    if(val == GC_FGD || val == GC_PR_FGD) // pixel malato (foreground)
                    {
                        utilita::setPixelValue(y,x,PelleSanaPreview,0,0,0);
                    }
                    else if(val == GC_BGD || val == GC_PR_BGD) // Pixel sano (background)
                    {
                        utilita::setPixelValue(y,x,PelleMalataPreview,0,0,0);
                    }
                }
            }
            auto t2 = Clock::now();
            auto delta =  std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            ui->TempoLabel->setText(QString::number((double)delta/1000));

            // Aggiorna le anteprime
            updatePreview(PelleSanaPreview,2);
            updatePreview(PelleMalataPreview,3);
        }

        ui->progressBar_2->setValue(100);
        segnaOccupato(0);
    }
    else
    {
        if(ui->usoProgrammaBox->currentText() == QString::fromStdString("Medico"))
        {
            utilita::showAlert(QString::fromStdString("Mancano marcature per la zona malata."));
        }
        else if(ui->usoProgrammaBox->currentText() == QString::fromStdString("Generale"))
        {
            QString classBlabel = ui->nomeSecondaClasseLabel->text();
            utilita::showAlert(QString::fromStdString("Mancano marcature per la classe ")+classBlabel);
        }
    }
}

void MainWindow::init(bool costruisci_mask = 1)
{
    abilitaVociMenu(0);

    if(!AnteprimaMarcature_current.data)
        AnteprimaMarcature_current = ImmagineOriginale.clone();
    if(!PelleSanaPreview.data)
        PelleSanaPreview = ImmagineOriginale.clone();
    if(!PelleMalataPreview.data)
        PelleMalataPreview = ImmagineOriginale.clone();

    // memorizzo la dimensione dell'immagine
    image_width = ImmagineOriginale.cols;
    image_heigth = ImmagineOriginale.rows;

    if(costruisci_mask)
    {
        // Costruisco la matrice delle classi dei pixel
        grabcut_mask_current = cv::Mat::ones(image_heigth,image_width,CV_8UC1);
        grabcut_mask_current = grabcut_mask_current * GC_PR_BGD;
    }

    grabcut_mask_undo = grabcut_mask_current.clone();
    grabcut_mask_redo = grabcut_mask_current.clone();

    // Anteprima delle marcature
    possoDisegnareLinea = 1; // Abilito la marcatura

    // aggiorno i valori di scala
    x_scale = (double) image_width / ui->OriginalImageLabel->width();
    y_scale = (double) image_heigth / ui->OriginalImageLabel->height();

    QString infoImage = QString::number(image_width)+QString::fromStdString("x")+QString::number(image_heigth);
    ui->dettagliImmagineLabel->setText(infoImage);


    segnaOccupato(1);
    ui->progressBar_2->setValue(0);
    cv::Mat foreground(ImmagineOriginale.size(),CV_8UC3,cv::Scalar(0,0,0));
    cv::Mat background(ImmagineOriginale.size(),CV_8UC3,cv::Scalar(0,0,0));
    ui->progressBar_2->setValue(10);

    // prepara le due anteprime della segmentazione
    ui->progressBar_2->setValue(75);
    if(!AnteprimaMarcature_current.data)
        AnteprimaMarcature_current = ImmagineOriginale.clone();

    aggiornaAnteprime();
    updatePreview(AnteprimaMarcature_current,1);

    ui->progressBar_2->setValue(90);

    // Aggiorno la dimensione massima del marcatura

    ui->brushSizeSelector->setMaximum((int)std::min(ImmagineOriginale.rows,ImmagineOriginale.cols)/2);

    ui->progressBar_2->setValue(100);
    segnaOccupato(0);


}

void MainWindow::segnaOccupato(bool value)
{
    occupato = value;
    if(value)
    {
        ui->segmentazione_in_corso_label1->setText(QString::fromStdString("Segmentazione in corso..."));
        ui->segmentazione_in_corso_label2->setText(QString::fromStdString("Segmentazione in corso..."));
    }
    else
    {
        ui->segmentazione_in_corso_label1->setText(QString::fromStdString(""));
        ui->segmentazione_in_corso_label2->setText(QString::fromStdString(""));
    }

    abilitaVociMenu(!value);
}

void MainWindow::on_actionSegna_pelle_sana_triggered()
{
    _classeCorrente = GC_BGD;
    ui->pixelClassLabel->setText("Pelle sana");
    ui->actionDisplayCurrentMarcature->setIcon(QIcon(":/icons/quadratoVerde"));
}

void MainWindow::abilitaVociMenu(bool value)
{
    ui->actionApplica_segmentazione->setEnabled(value);
    ui->actionSegna_pelle_sana->setEnabled(value);
    ui->actionSegna_pelle_malata->setEnabled(value);
    ui->action_zoom_image->setEnabled(value);
    ui->actionZoomOut->setEnabled(value);
    ui->actionSelettore_Intelligente->setEnabled(value);
    ui->actionAnnulla->setEnabled(value);
    ui->actionRipeti_Marcatura->setEnabled(value);
    ui->salvaAnteprimaMarcatureButton->setEnabled(value);
    ui->salvaPelleSanaPreviewButton->setEnabled(value);
    ui->salvaPelleMalataPreviewButton->setEnabled(value);
    ui->actionApprendi_da_segmentazione_precedente->setEnabled(value);
    ui->actionEqualizzazioneIstogramma->setEnabled(value);
    ui->actionAbilitaFastGrabcut->setEnabled(value);
    ui->actionSegmentazione_continua->setEnabled(value);
    ui->actionSalva_risultati->setEnabled(value);
    ui->actionDisplayCurrentMarcature->setEnabled(value);
    ui->actionProvaAlgoritmo->setEnabled(value);
    ui->actionApplica_filtro_personalizzato->setEnabled(value);
    ui->actionBrushDOWN->setEnabled(value);
    ui->actionBrushUP->setEnabled(value);
}

void MainWindow::on_action_zoom_image_triggered()
{

    ui->OriginalImageLabel->setFixedHeight(image_heigth);
    ui->OriginalImageLabel->setFixedWidth(image_width);
    ui->scrollAreaWidgetContents_2->adjustSize();

    y_scale = 1;
    x_scale = 1;
}

// Quando l'immagine è ridimensionata al minimo
void MainWindow::on_actionZoomOut_triggered()
{
    updatePreview(AnteprimaMarcature_current,1);
    ui->OriginalImageLabel->setFixedHeight(460);
    ui->OriginalImageLabel->setFixedWidth(300);
    ui->scrollAreaWidgetContents_2->adjustSize();
    x_scale = (double) image_width / ui->OriginalImageLabel->width();
    y_scale = (double) image_heigth / ui->OriginalImageLabel->height();
}

// Cambio del valore nello slider dello zoom
void MainWindow::on_zoomSlider_valueChanged(int value)
{
    // cambia zoom immagine
    ui->zoomValue->setText(QString::number(value));
    x_scale = (double) 100 / value;
    y_scale = (double) 100 / value;

    if((int)image_heigth/y_scale < 460 || (int)image_width/x_scale < 300)
    {
        on_actionZoomOut_triggered();
    }
    else
    {
        ui->OriginalImageLabel->setFixedHeight((int)image_heigth/y_scale);
        ui->OriginalImageLabel->setFixedWidth((int)image_width/x_scale);
    }
    ui->scrollAreaWidgetContents_2->adjustSize();


}

void MainWindow::on_actionAbilitaFastGrabcut_triggered()
{
    if(usaFastGrabcut)
    {
        usaFastGrabcut = 0;
        ui->actionAbilitaFastGrabcut->setToolTip(QString::fromStdString("Abilita la segmentazione veloce"));
    }
    else
    {
        usaFastGrabcut = 1;
        ui->actionAbilitaFastGrabcut->setToolTip(QString::fromStdString("Disabilita la segmentazione veloce"));
    }
}

void MainWindow::on_luminositaSlider_valueChanged(int value)
{
     ui->luminositaValue->setText(QString::number(value));

    ImmagineOutput = ImmagineOriginale.clone();
    utilita::modificaLuminositaContrasto(ImmagineOriginale,ImmagineOutput,1,value);

    // Integro le marcature e salvo su Anteprima Marcature

    grabcut_mask_undo = grabcut_mask_current.clone();
    AnteprimaMarcature_undo = AnteprimaMarcature_current.clone();
    AnteprimaMarcature_current = ImmagineOutput.clone();


    for(int y = 0; y < image_heigth; y ++)
    {
        for(int x = 0; x < image_width; x ++)
        {
            uchar val = grabcut_mask_current.at<uchar>(y,x);
            if(val == GC_FGD) // pixel malato (foreground)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,0,255);
            }
            else if(val == GC_BGD) // Pixel sano (background)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,255,0);
            }
        }
    }

    ui->actionAnnulla->setEnabled(true);
    ui->actionRipristina_immagine_originale->setEnabled(true);
    updatePreview(AnteprimaMarcature_current,1);


    updatePreview(AnteprimaMarcature_current,1);
}

void MainWindow::on_contrastoSlider_valueChanged(int value)
{

    ui->constrastoValue->setText(QString::number(value));

    ImmagineOutput = ImmagineOriginale.clone();
    utilita::modificaLuminositaContrasto(ImmagineOriginale,ImmagineOutput,value,0);
    // Integro le marcature e salvo su Anteprima Marcature

    grabcut_mask_undo = grabcut_mask_current.clone();
    AnteprimaMarcature_undo = AnteprimaMarcature_current.clone();
    AnteprimaMarcature_current = ImmagineOutput.clone();


    for(int y = 0; y < image_heigth; y ++)
    {
        for(int x = 0; x < image_width; x ++)
        {
            uchar val = grabcut_mask_current.at<uchar>(y,x);
            if(val == GC_FGD) // pixel malato (foreground)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,0,255);
            }
            else if(val == GC_BGD) // Pixel sano (background)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,255,0);
            }
        }
    }

    ui->actionAnnulla->setEnabled(true);
    ui->actionRipristina_immagine_originale->setEnabled(true);

    updatePreview(AnteprimaMarcature_current,1);


    updatePreview(AnteprimaMarcature_current,1);
}

void MainWindow::on_actionSegmentazione_continua_triggered()
{
    segmentazione_continua = !segmentazione_continua;
    ui->actionSegmentazione_continua->setChecked(segmentazione_continua);
}

void MainWindow::on_actionSalva_risultati_triggered()
{
    QString nomeCognome = ui->nomeCognomeLabel->text();
    QString data = ui->dateEdit->date().toString();
    QString patologia = ui->patologiaLabel->text();
    QString livello = QString::number(ui->livelloPatologiaSpinBox->value());
    QString fototipo = QString::number(ui->fototipoSpinBox->value());
    QString suggestedFileName = nomeCognome + QString::fromStdString(" ") + data;
    suggestedFileName = suggestedFileName.replace(" ","_"); // togliamo gli spazi bianchi
    QString fileName = QFileDialog::getSaveFileName(this, tr("Salva file"),
                                                    suggestedFileName,
                                                    tr("Tutti i file (*.*)"));
    if(!fileName.isEmpty())
    {
        utilita::esportaAnagrafica(fileName,nomeCognome,data,patologia,livello,fototipo);
        utilita::esportaData(fileName,ImmagineOriginale,AnteprimaMarcature_current,PelleSanaPreview,PelleMalataPreview,grabcut_mask_current);
        utilita::showAlert(QString::fromStdString("Esportazione effettuata!"));
    }
}

void MainWindow::on_nIterazioniSlider_valueChanged(int value)
{
    ui->nIterazioniLabel->setText(QString::number(ui->nIterazioniSlider->value()));
}

void MainWindow::on_actionCarica_un_lavoro_precedente_triggered()
{
    // importo l'immagine da disco

    QString fileNameTXT = QFileDialog::getOpenFileName(this,tr("Apri file di anagrafica"),QDir::currentPath(),tr("File di anagrafica (*.txt)"));
    char* charFileNameTXT = fileNameTXT.toLocal8Bit().data();

    if(QFile::exists(charFileNameTXT))
    {
        // leggi da file txt
        string STRING;
        ifstream infile;
        infile.open (charFileNameTXT);

        // parsing del nome e del cognome
        getline(infile,STRING);
        getline(infile,STRING); // Saves the line in STRING.

        if(!STRING.empty())
        {
            std::cout << STRING << std::endl;
            ui->nomeCognomeLabel->setText(QString::fromStdString(STRING));
        }
        // parsing della data del referto
        getline(infile,STRING);
        getline(infile,STRING); // Saves the line in STRING.

        if(!STRING.empty())
        {
            std::cout << STRING << std::endl;
            ui->dateEdit->setDate(QDate::fromString(QString::fromStdString(STRING)));
        }
        // parsing della patologia
        getline(infile,STRING);
        getline(infile,STRING); // Saves the line in STRING.

        if(!STRING.empty())
        {
            std::cout << STRING << std::endl;
            ui->patologiaLabel->setText(QString::fromStdString(STRING));
        }
        // parsing del livello
        getline(infile,STRING);
        getline(infile,STRING); // Saves the line in STRING.

        if(!STRING.empty())
        {
            std::cout << STRING << std::endl;
            int livello = atoi(STRING.c_str());
            ui->livelloPatologiaSpinBox->setValue(livello);
        }

        // parsing del fototipo
        getline(infile,STRING);
        getline(infile,STRING); // Saves the line in STRING.
        if(!STRING.empty())
        {
            int fototipo = atoi(STRING.c_str());
            ui->fototipoSpinBox->setValue(fototipo);
        }

        infile.close();
    }


    QString fileName = QFileDialog::getOpenFileName(this,tr("Apri salvataggio Matlab"),QDir::currentPath(),tr("File di salvataggio MATLAB (*.mat)"));
    char* charFileName = fileName.toLocal8Bit().data();

    if(QFile::exists(charFileName))
    {
        cv::FileStorage storage(charFileName, cv::FileStorage::READ);
        storage["image"] >> ImmagineOriginale;
        storage["anteprima_marcature"] >> AnteprimaMarcature_current;
        storage["anteprima_sana"] >> PelleSanaPreview;
        storage["anteprima_malata"] >> PelleMalataPreview;
        storage["mask"] >> grabcut_mask_current;
        storage.release();
        if(ImmagineOriginale.data)// Check for invalid input
        {
            ui->nomeFileLabel->setText(QString::fromStdString(charFileName));
            init(0); // mostrare anteprima marcature
        }
    }
}

void MainWindow::on_fattoreVelocitaSlider_valueChanged(int value)
{
    fattoreScala = value;
    if(fattoreScala == 1)
    {
        // devo disabilitare la modalità fast
        usaFastGrabcut = 0;
        ui->actionAbilitaFastGrabcut->setChecked(false);
    }
    else
    {
        // devo abilitare la modalità fast
        usaFastGrabcut = 1;
        ui->actionAbilitaFastGrabcut->setChecked(true);
        ui->fattoreVelocitaLabel->setText(QString::number(fattoreScala));
    }
}

// Esci dal software
void MainWindow::on_actionEsci_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Uscita in corso", "Potresti avere modifiche non salvate. Sei sicuro?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {

        // Conserva alcune impostazioni

        cv::FileStorage storage("settings.mat", cv::FileStorage::WRITE);
        storage << "fattore_velocizzazione" << ui->fattoreVelocitaSlider->value();
        storage << "nIterazioni" << ui->nIterazioniSlider->value();
        storage << "brushSize" << ui->brushSizeSelector->value();
        storage << "fattoreColorizzazione" << fattore_colorizzazione;

        storage.release();

        // Libera la memoria
        resetVariabili();
        QApplication::quit();
    }
    else if (reply == QMessageBox::No)
    {
        // rimani qui
        std::cout << "Ho annullato " << std::endl;
    }
}

// Consente di selezionare con un rettangolo un'area malata e far partire grabcut su questa sottomatrice
void MainWindow::on_actionSelettore_Intelligente_triggered()
{
    selettoreIntelligenteMode = 1;
}

void MainWindow::ripristinaImpostazioni()
{
    cv::FileStorage storage("settings.mat", cv::FileStorage::READ);

    int fVel, nIter, tmpBrushSize;
    storage["fattore_velocizzazione"] >> fVel;
    if(fVel>0) ui->fattoreVelocitaSlider->setValue(fVel);
    storage["nIterazioni"] >> nIter;
    if(fVel>0) ui->nIterazioniSlider->setValue(nIter);
    storage["brushSize"] >> tmpBrushSize;
    if(tmpBrushSize>0) {
        ui->brushSizeSelector->setValue(tmpBrushSize);
        brush_size = tmpBrushSize;
    }

    storage["fattoreColorizzazione"] >> fattore_colorizzazione;
    storage.release();
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    on_actionEsci_triggered();
}

void MainWindow::on_fattoreColorizzazioneSlider_valueChanged(int value)
{
  fattore_colorizzazione = value;
  ui->fattoreColorizzazioneLabel->setText(QString::number(fattore_colorizzazione));
}

void MainWindow::on_actionAnnulla_triggered()
{
    grabcut_mask_redo = grabcut_mask_current.clone();
    grabcut_mask_current = grabcut_mask_undo.clone();

    AnteprimaMarcature_redo = AnteprimaMarcature_current.clone();
    AnteprimaMarcature_current = AnteprimaMarcature_undo.clone();
    updatePreview(AnteprimaMarcature_current,1);
    //aggiornaAnteprimaMarcature();
    ui->actionRipeti_Marcatura->setEnabled(true);
    ui->actionAnnulla->setEnabled(false);
}

void MainWindow::on_actionRipeti_Marcatura_triggered()
{
    grabcut_mask_undo = grabcut_mask_current.clone();
    grabcut_mask_current = grabcut_mask_redo.clone();
    AnteprimaMarcature_undo = AnteprimaMarcature_current.clone();
    AnteprimaMarcature_current = AnteprimaMarcature_redo.clone();
    updatePreview(AnteprimaMarcature_current,1);
    //aggiornaAnteprimaMarcature();
    ui->actionRipeti_Marcatura->setEnabled(false);
    ui->actionAnnulla->setEnabled(true);
}

void MainWindow::on_nomePrimaClasseLabel_textChanged(const QString &arg1)
{
    ui->label_2->setText(arg1);
}

void MainWindow::on_nomeSecondaClasseLabel_textChanged(const QString &arg1)
{
    ui->label_3->setText(arg1);
}

void MainWindow::on_usoProgrammaBox_currentTextChanged(const QString &arg1)
{
    if(arg1 == QString::fromStdString("Medico"))
    {
        ui->tab_3->setEnabled(true);
    }
    else if(arg1 == QString::fromStdString("Generale"))
    {
        ui->tab_3->setEnabled(false);
    }
}

void MainWindow::resetVariabili()
{

    ui->OriginalImageLabel->clear();

    ui->PelleMalata_LabelImg->clear();
    ui->PelleSana_LabelImg->clear();

    abilitaVociMenu(false);


    if(grabcut_mask_current.data)
        grabcut_mask_current.release();

    if(grabcut_mask_redo.data)
        grabcut_mask_redo.release();

    if(grabcut_mask_undo.data)
        grabcut_mask_undo.release();


    if(AnteprimaMarcature_current.data)
        AnteprimaMarcature_current.release();

    if(AnteprimaMarcature_undo.data)
        AnteprimaMarcature_undo.release();

    if(AnteprimaMarcature_redo.data)
        AnteprimaMarcature_redo.release();


    if(PelleMalataPreview.data)
        PelleMalataPreview.release();

    if(PelleSanaPreview.data)
        PelleSanaPreview.release();
}

void MainWindow::on_salvaPelleMalataPreviewButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Salva file"),
                                                    "AnteprimaPelleMalata.jpg",
                                                    tr("Immagine JPG (*.jpg)"));
    cv::imwrite(fileName.toStdString(), PelleMalataPreview );
}

void MainWindow::on_salvaPelleSanaPreviewButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Salva file"),
                                                    "AnteprimaPelleSana.jpg",
                                                    tr("Immagine JPG (*.jpg)"));
    cv::imwrite(fileName.toStdString(), PelleSanaPreview );
}

void MainWindow::on_salvaAnteprimaMarcatureButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Salva file"),
                                                    "AnteprimaMarcature.jpg",
                                                    tr("Immagine JPG (*.jpg)"));
    cv::imwrite(fileName.toStdString(), AnteprimaMarcature_current );
}

void MainWindow::on_actionApprendi_da_segmentazione_precedente_triggered()
{



    // Variabili temporaneae

    cv::Mat img_vecchia = ImmagineOriginale.clone();
    cv::Mat mask_vecchia = grabcut_mask_current.clone();

    // importo l'immagine da disco

    QString fileName = QFileDialog::getOpenFileName(this,tr("Apri segmentazione"),QDir::currentPath(),tr("Tutti i file (*.*)"));
    char* charFileName = fileName.toLocal8Bit().data();

    if(QFile::exists(charFileName))
    {
        cv::FileStorage storage(charFileName, cv::FileStorage::READ);
        storage["image"] >> img_vecchia;
        storage["mask"] >> mask_vecchia;
        storage.release();
        if(img_vecchia.data)// Check for invalid input
        {

            // Adatta la maschera
            utilita::showAlert(QString::fromStdString("Le dimensioni non combaciano. La maschera verrà scalata\nalle dimensioni dell'immagine attualmente caricata ma\nil risultato potrebbe mancare di accuratezza."));

            cv::resize(img_vecchia, img_vecchia, ImmagineOriginale.size(),cv::INTER_NEAREST);
            cv::resize(mask_vecchia, mask_vecchia, grabcut_mask_current.size(),cv::INTER_NEAREST);

            utilita::creaMaskDaConfronto(img_vecchia,ImmagineOriginale,mask_vecchia,grabcut_mask_current);
            on_actionApplica_segmentazione_triggered();
        }
    }


}

void MainWindow::on_actionEqualizzazioneIstogramma_triggered()
{
    cv::Mat dst = ImmagineOriginale.clone();

    cv::Mat ycrcb;
    cvtColor(ImmagineOriginale,ycrcb,CV_BGR2YCrCb);
    vector<cv::Mat> channels;
    split(ycrcb,channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels,ycrcb);
    cvtColor(ycrcb,dst,CV_YCrCb2BGR);

    updatePreview(dst,1);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Operazione effettuata!", "Accetti le modifiche visuali?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        ImmagineOriginale = dst.clone();
    }
    else if (reply == QMessageBox::No)
    {
        dst.release();
        updatePreview(ImmagineOriginale,1);
    }

}

void MainWindow::aggiornaAnteprime()
{
    // aggiorno le anteprime
    PelleSanaPreview = ImmagineOriginale.clone();
    PelleMalataPreview = ImmagineOriginale.clone();

    for(int y = 0; y < image_heigth; y ++)
    {
        for(int x = 0; x < image_width; x ++)
        {
            uchar val = grabcut_mask_current.at<uchar>(y,x);
            if(val == GC_FGD || val == GC_PR_FGD) // pixel malato (foreground)
            {
                utilita::setPixelValue(y,x,PelleSanaPreview,0,0,0);
            }
            else if(val == GC_BGD || val == GC_PR_BGD) // Pixel sano (background)
            {
                utilita::setPixelValue(y,x,PelleMalataPreview,0,0,0);
            }
        }
    }
    updatePreview(PelleSanaPreview,2);
    updatePreview(PelleMalataPreview,3);
}

void MainWindow::on_actionAbout_triggered()
{
    utilita::showAlert(QString::fromStdString("Software creato da\nIlluminato Luca Costantino. L'uso è libero ai fini non commerciali."));
}

// CONSENTE DI SEGMENTARE USANDO UN ALTRO ALGORITMO
// L'IMPORTANTE È CONVERTIRE LA MASCHERA FINALE NEL FORMATO DI GRABCUT (0,1,2,3)
void MainWindow::on_actionProvaAlgoritmo_triggered()
{
    segnaOccupato(true);

    cv::Mat resizedImage = cv::Mat::ones((int) image_heigth / 16,(int) image_width / 16,CV_8UC1);
    cv::Mat resizedMask = cv::Mat::ones((int) image_heigth / 16,(int) image_width / 16,CV_8UC1);
    utilita::campiona(ImmagineOriginale,resizedImage);
    utilita::campiona(grabcut_mask_current,resizedMask);
    cv::Mat dst = resizedMask.clone();
    utilita::testAlgoritmo(ImmagineOriginale,resizedMask,dst);
    utilita::replication(dst,grabcut_mask_current);

    // elimino le strutture inutili
    dst.release();
    resizedImage.release();
    resizedMask.release();

    aggiornaAnteprime();

    segnaOccupato(false);
}

void MainWindow::on_actionApplica_filtro_personalizzato_triggered()
{
    cv::Mat* customKernel = new cv::Mat(3, 3, CV_64F);

    // Popoliamo il kernel leggendo dalle spinbox

    customKernel->at<double>(0,0) = ui->kernel00->value();
    customKernel->at<double>(0,1) = ui->kernel01->value();
    customKernel->at<double>(0,2) = ui->kernel02->value();
    customKernel->at<double>(1,0) = ui->kernel10->value();
    customKernel->at<double>(1,1) = ui->kernel11->value();
    customKernel->at<double>(1,2) = ui->kernel12->value();
    customKernel->at<double>(2,0) = ui->kernel20->value();
    customKernel->at<double>(2,1) = ui->kernel21->value();
    customKernel->at<double>(2,2) = ui->kernel22->value();

    ImmagineOutput = ImmagineOriginale.clone();
    utilita::applyMask(ImmagineOriginale,ImmagineOutput,customKernel);

    // Integro le marcature e salvo su Anteprima Marcature

    grabcut_mask_undo = grabcut_mask_current.clone();
    AnteprimaMarcature_undo = AnteprimaMarcature_current.clone();
    AnteprimaMarcature_current = ImmagineOutput.clone();


    for(int y = 0; y < image_heigth; y ++)
    {
        for(int x = 0; x < image_width; x ++)
        {
            uchar val = grabcut_mask_current.at<uchar>(y,x);
            if(val == GC_FGD) // pixel malato (foreground)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,0,255);
            }
            else if(val == GC_BGD) // Pixel sano (background)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,255,0);
            }
        }
    }

    ui->actionAnnulla->setEnabled(true);
    ui->actionRipristina_immagine_originale->setEnabled(true);
    updatePreview(AnteprimaMarcature_current,1);


}

void MainWindow::on_actionConferma_modifiche_all_immagine_originale_triggered()
{
    ImmagineOriginale = ImmagineOutput.clone();
    ui->actionRipristina_immagine_originale->setEnabled(false);
}

void MainWindow::on_actionRipristina_immagine_originale_triggered()
{
    AnteprimaMarcature_current = ImmagineOriginale.clone();
    for(int y = 0; y < image_heigth; y ++)
    {
        for(int x = 0; x < image_width; x ++)
        {
            uchar val = grabcut_mask_current.at<uchar>(y,x);
            if(val == GC_FGD) // pixel malato (foreground)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,0,255);
            }
            else if(val == GC_BGD) // Pixel sano (background)
            {
                utilita::setPixelValue(y,x,AnteprimaMarcature_current,0,255,0);
            }
        }
    }
    ui->actionRipristina_immagine_originale->setEnabled(false);
    updatePreview(AnteprimaMarcature_current,1);
}

void MainWindow::on_actionBrushUP_triggered()
{
    int _brushSize = ui->brushSizeSelector->value();
    ui->brushSizeSelector->setValue(_brushSize +1);
    brush_size = _brushSize + 1;
}

void MainWindow::on_actionBrushDOWN_triggered()
{
    int _brushSize = ui->brushSizeSelector->value();
    ui->brushSizeSelector->setValue(_brushSize -1);
    brush_size = _brushSize - 1;

}
