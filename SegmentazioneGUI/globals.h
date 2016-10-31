#ifndef GLOBALS_H
#define GLOBALS_H

#include "includes.h"

// DISEGNO
extern bool possoDisegnareLinea; // abilita eventi mousee
extern int _classeCorrente; // variabile temporanea per la classe corrente di marcatura
extern double x_scale;
extern double y_scale; // rapporto tra la dimensione reale dell'immagine e il container (y)
extern int fattore_colorizzazione;
extern int brush_size; // dimensione pennello di marcatura

// STRUTTURE DATI
extern cv::Mat AnteprimaMarcature_current, AnteprimaMarcature_undo, AnteprimaMarcature_redo;
extern cv::Mat ImmagineOutput;
extern cv::Mat grabcut_mask_current, grabcut_mask_undo, grabcut_mask_redo;

// ALTRE VARIABILI
extern int image_width; // larghezza reale immagine
extern int image_heigth; // lunghezza reale immagine
extern bool occupato, selettoreIntelligenteMode;
extern int bBox_XStart, bBox_YStart, bBox_XEnd, bBox_YEnd; // angoli boundingbox

#endif // GLOBALS_H
