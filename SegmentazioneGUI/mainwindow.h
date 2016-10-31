#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define QLABEL_WIDTH 320
#define QLABEL_HEIGHT 480

// Inclusioni mie

#include "mymouseqlabel.h"
#include <QDialog>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);

        ~MainWindow();

    private slots:

        void on_actionOpen_image_triggered();
        void on_actionSegna_pelle_sana_triggered();
        void on_actionSegna_pelle_malata_triggered();
        void on_brushSizeSelector_valueChanged(int value);
        void on_actionApplica_segmentazione_triggered();
        void on_actionZoomOut_triggered();
        void on_action_zoom_image_triggered();
        void on_zoomSlider_valueChanged(int value);
        void on_actionAbilitaFastGrabcut_triggered();
        void on_luminositaSlider_valueChanged(int value);
        void on_contrastoSlider_valueChanged(int value);
        void on_actionSegmentazione_continua_triggered();
        void on_actionSalva_risultati_triggered();
        void on_nIterazioniSlider_valueChanged(int value);
        void on_actionCarica_un_lavoro_precedente_triggered();
        void on_fattoreVelocitaSlider_valueChanged(int value);
        void on_actionEsci_triggered();
        void on_actionSelettore_Intelligente_triggered();
        void on_fattoreColorizzazioneSlider_valueChanged(int value);
        void on_actionAnnulla_triggered();
        void on_actionRipeti_Marcatura_triggered();
        void on_nomePrimaClasseLabel_textChanged(const QString &arg1);
        void on_nomeSecondaClasseLabel_textChanged(const QString &arg1);
        void on_usoProgrammaBox_currentTextChanged(const QString &arg1);
        void on_salvaPelleMalataPreviewButton_clicked();
        void on_salvaPelleSanaPreviewButton_clicked();
        void on_salvaAnteprimaMarcatureButton_clicked();
        void on_actionApprendi_da_segmentazione_precedente_triggered();
        void on_actionEqualizzazioneIstogramma_triggered();
        void on_actionAbout_triggered();
        void on_actionProvaAlgoritmo_triggered();
        void on_actionApplica_filtro_personalizzato_triggered();
        void on_actionConferma_modifiche_all_immagine_originale_triggered();
        void on_actionRipristina_immagine_originale_triggered();
        void on_actionBrushUP_triggered();
        void on_actionBrushDOWN_triggered();

        // Metodi di supporto

        void updatePreview(cv::Mat &mat, int where);
        void init(bool costruisci_mask);
        void segnaOccupato(bool value);
        void abilitaVociMenu(bool value);
        void ripristinaImpostazioni();
        void aggiornaAnteprime();
        void resetVariabili();

        // Eventi

        void MouseLeft_Received();
        void MousePos_Received();
        void closeEvent (QCloseEvent *event);



private:
        Ui::MainWindow *ui;
        cv::Mat ImmagineOriginale, PelleSanaPreview, PelleMalataPreview;
        bool segmentazione_continua, usaFastGrabcut;
        short fattoreScala = 1;
};


#endif // MAINWINDOW_H
