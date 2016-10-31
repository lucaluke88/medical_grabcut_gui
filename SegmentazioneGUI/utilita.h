#ifndef UTILITA_H
#define UTILITA_H

#include "estensioni.h"

namespace utilita
{
    static void setPixelValue(int y, int x, cv::Mat &img, int valueB, int valueG, int valueR)
    {
        img.at<cv::Vec3b>(y,x).val[0] = valueB;
        img.at<cv::Vec3b>(y,x).val[1] = valueG;
        img.at<cv::Vec3b>(y,x).val[2] = valueR;
    }

    static cv::Vec3b getPixelValue(int y, int x, cv::Mat &img)
    {
        return img.at<cv::Vec3b>(y, x);
    }



    static void campiona(cv::Mat &input, cv::Mat &output)
    {
        // input: matrice originale, output matrice a metà
        resize(input, output, output.size(), 0, 0, cv::INTER_NEAREST);
    }

    static void replication(cv::Mat &input, cv::Mat &output)
    {
        resize(input, output, output.size(), 0, 0, cv::INTER_NEAREST);
    }

    static void modificaLuminositaContrasto(cv::Mat &image,cv::Mat &output,int constrasto=1, int luminosita=0)
    {
        image.convertTo(output, -1, constrasto, luminosita);
    }

    static void applyMask(cv::Mat &image, cv::Mat &output,cv::Mat* kern)
    {
        cv::filter2D(image, output, image.depth(), *kern);
    }

    static void myGrabCut(cv::Mat image, cv::Mat &mask, cv::Mat &foreground,cv::Mat &background, short nIterazioni=1,bool useMask = 1, bool fastMode=0,int fattoreScala=1, bool isInit=1)
    {
        occupato = 1;
        if(fastMode)
        {
            if(useMask) // fast + mask
            {
                cv::Mat resizedImage = cv::Mat::ones((int) image_heigth / fattoreScala,(int) image_width / fattoreScala,CV_8UC1);
                cv::Mat resizedMask = cv::Mat::ones((int) image_heigth / fattoreScala,(int) image_width / fattoreScala,CV_8UC1);
                campiona(image,resizedImage);
                campiona(mask,resizedMask);
                // define bounding rectangle
                cv::Rect rectangle(2,2,resizedImage.cols-2,resizedImage.rows-2);
                // segmentation result (4 possible values)
                cv::Mat bgModel,fgModel; // the models (internally used)
                // GrabCut segmentation
                cv::grabCut(resizedImage,    // input image
                            resizedMask,   // segmentation result
                            rectangle,// rectangle containing foreground
                            bgModel,fgModel, // models
                            1,        // number of iterations
                            cv::GC_INIT_WITH_MASK); // use mask
                // Get the pixels marked as likely foreground
                cv::compare(resizedMask,cv::GC_PR_FGD,foreground,cv::CMP_EQ);
                // Get the pixels marked as likely background
                cv::compare(resizedMask,cv::GC_PR_BGD,background,cv::CMP_EQ);
                replication(resizedMask,mask);
            }
            else // fast + rect
            {
                cv::Mat resizedImage = cv::Mat::ones((int) image_heigth / 16,(int) image_width / 16,CV_8UC1);
                cv::Mat resizedMask = cv::Mat::ones((int) image_heigth / 16,(int) image_width / 16,CV_8UC1);

                campiona(image,resizedImage);
                campiona(mask,resizedMask);

                // define bounding rectangle
                cv::Rect rectangle(2,2,resizedImage.cols-2,resizedImage.rows-2);

                ; // segmentation result (4 possible values)
                cv::Mat bgModel,fgModel; // the models (internally used)

                // GrabCut segmentation
                cv::grabCut(resizedImage,    // input image
                            resizedMask,   // segmentation result
                            rectangle,// rectangle containing foreground
                            bgModel,fgModel, // models
                            1,        // number of iterations
                            cv::GC_INIT_WITH_RECT); // use rectangle

                // Get the pixels marked as likely foreground
                cv::compare(resizedMask,cv::GC_PR_FGD,foreground,cv::CMP_EQ);
                // Get the pixels marked as likely background
                cv::compare(resizedMask,cv::GC_PR_BGD,background,cv::CMP_EQ);


                if(isInit)
                {
                    // Scambio i valori della mask (primo avvio)
                    for(int y=0;y<resizedMask.rows;y++)
                    {
                        for(int x=0;x<resizedMask.cols;x++)
                        {
                            if(resizedMask.at<uchar>(y,x)==cv::GC_PR_FGD)
                                resizedMask.at<uchar>(y,x) = cv::GC_PR_BGD;
                            else if(resizedMask.at<uchar>(y,x)==cv::GC_PR_BGD)
                                resizedMask.at<uchar>(y,x) = cv::GC_PR_FGD;
                            else if(resizedMask.at<uchar>(y,x)==cv::GC_FGD)
                                resizedMask.at<uchar>(y,x) = cv::GC_BGD;
                            else    resizedMask.at<uchar>(y,x) = cv::GC_FGD;
                        }
                    }
                }

                replication(resizedMask,mask);
            }
        }
        else
        {
            if(useMask) // normale + mask
            {
                // define bounding rectangle
                cv::Rect rectangle(2,2,image.cols-2,image.rows-2);

                // segmentation result (4 possible values)
                cv::Mat bgModel,fgModel; // the models (internally used)

                // GrabCut segmentation
                cv::grabCut(image,    // input image
                            mask,   // segmentation result
                            rectangle,// rectangle containing foreground
                            bgModel,fgModel, // models
                            1,        // number of iterations
                            cv::GC_INIT_WITH_MASK); // use mask

                // Get the pixels marked as likely foreground
                cv::compare(mask,cv::GC_PR_FGD,foreground,cv::CMP_EQ);
                // Get the pixels marked as likely background
                cv::compare(mask,cv::GC_PR_BGD,background,cv::CMP_EQ);

            }
            else // normale + rect
            {

                // define bounding rectangle
                cv::Rect rectangle(2,2,image.cols-2,image.rows-2);

                ; // segmentation result (4 possible values)
                cv::Mat bgModel,fgModel; // the models (internally used)

                // GrabCut segmentation
                cv::grabCut(image,    // input image
                            mask,   // segmentation result
                            rectangle,// rectangle containing foreground
                            bgModel,fgModel, // models
                            1,        // number of iterations
                            cv::GC_INIT_WITH_RECT); // use rectangle

                // Get the pixels marked as likely foreground
                cv::compare(mask,cv::GC_PR_FGD,foreground,cv::CMP_EQ);
                // Get the pixels marked as likely background
                cv::compare(mask,cv::GC_PR_BGD,background,cv::CMP_EQ);

            }
        }
        std::printf("segmentazione effettuata");
        occupato = 0;
    }


    static void localGrabcut(cv::Mat &image, cv::Mat &mask)
    {
        occupato = 1;
        // dentro il rettangolo di selezione dell'immagine, ne traccio virtualmente uno più piccolo che indica dove sia l'oggetto in foreground
        cv::Rect rectangle(5,5,image.cols-5,image.rows-5);

        ; // segmentation result (4 possible values)
        cv::Mat bgModel,fgModel; // the models (internally used)

        // GrabCut segmentation
        cv::grabCut(image,    // input image
                    mask,   // segmentation result
                    rectangle,// rectangle containing foreground
                    bgModel,fgModel, // models
                    1,        // number of iterations
                    cv::GC_INIT_WITH_RECT); // use rectangle

        occupato = 0;
    }

    static void esportaAnagrafica(QString filename, QString nomeCognome, QString data, QString patologia,QString livello,QString fototipo)
    {
        // Dati anagrafici
        QString filename_report = filename + QString::fromStdString("_anagrafica.txt");
        QFile file(filename_report);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream << "%% NOME E COGNOME %%" << endl;
            stream << nomeCognome << endl;
            stream << "%% DATA REFERTO %%" << endl;
            stream << data << endl;
            stream << "%% PATOLOGIA %%" << endl;
            stream << patologia << endl;
            stream << "%% LIVELLO %%" << endl;
            stream<< livello << endl;
            stream << "%% FOTOTIPO %%" << endl;
            stream << fototipo << endl;
            file.close();
        }
    }

    static void esportaData(QString filename, cv::Mat &immagine_originale,cv::Mat &anteprima_marcature,cv::Mat &pellesanapreview,cv::Mat &pellemalatapreview,cv::Mat &mask)
    {
        // Strutture dati
        cv::FileStorage storage(filename.toStdString()+"_data.mat", cv::FileStorage::WRITE);
        storage << "image" << immagine_originale;
        storage << "anteprima_marcature" << anteprima_marcature;
        storage << "anteprima_sana" << pellesanapreview;
        storage << "anteprima_malata" << pellemalatapreview;
        storage << "mask" << mask;
        storage.release();

        QMessageBox messageBox;
        messageBox.setText(QString::fromStdString("Salvataggio effettuato!"));
        messageBox.setFixedSize(500,200);
    }

    static int percentualePelleMalata(cv::Mat &mask)
    {
        int sum = 0;
        for(int y=0;y<image_heigth;y++)
        {
            for(int x=0;x<image_width;x++)
            {
                if(mask.at<uchar>(y,x)==cv::GC_FGD || mask.at<uchar>(y,x)==cv::GC_PR_FGD)
                    ++sum;
            }
        }
        return sum * 100 /(image_heigth * image_width);
    }


    static void creaMaskDaConfronto(cv::Mat &img_orig_vecchia, cv::Mat &img_orig_nuova, cv::Mat &mask_vecchia, cv::Mat &mask_nuova)
    {
        mask_nuova = mask_vecchia.clone(); // così modifichiamo solo i valori diversi
        for(int y=0;y<mask_nuova.rows;y++)
        {
            for(int x=0;x<mask_nuova.cols;x++)
            {
                if(img_orig_vecchia.at<uchar>(y,x) != img_orig_nuova.at<uchar>(y,x))
                {
                    if(mask_vecchia.at<uchar>(y,x) == cv::GC_BGD || mask_vecchia.at<uchar>(y,x) == cv::GC_PR_BGD)
                        mask_nuova.at<uchar>(y,x) = cv::GC_PR_BGD;
                    else
                        mask_nuova.at<uchar>(y,x) = cv::GC_PR_FGD;

                }
            }
        }
    }

    static bool seedMalatoExists(cv::Mat &mask)
    {
        for(int y=0;y<mask.rows;y++)
        {
            for(int x=0;x<mask.cols;x++)
            {
                if(mask.at<uchar>(y,x)==cv::GC_FGD || mask.at<uchar>(y,x)==cv::GC_PR_FGD)
                    return true;
            }
        }
        return false;
    }

    static void showAlert(QString message)
    {
        QMessageBox msgBox;
        msgBox.setText(message);
        msgBox.exec();
    }

    // TEST DI ALTRI ALGORITMI - INTERFACCIA GENERALE
    static void testAlgoritmo(cv::Mat& src_image, cv::Mat& grabcut_mask, cv::Mat& dst)
    {
        bool usa_growcut = 1;
        if(usa_growcut)
        {
            growCut(src_image,grabcut_mask,dst);
        }
    }

// FINE DEI METODI
}

#endif // UTILITA_H
