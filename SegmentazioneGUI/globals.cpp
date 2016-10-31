#include "globals.h"

// VARIABILI DI DISEGNO

bool possoDisegnareLinea = 0;
bool selettoreIntelligenteMode = 0;
int _classeCorrente = 0;
int brush_size = 36;
int fattore_colorizzazione = 50;
double x_scale = 0;
double y_scale = 0;

// STRUTTURE PER CRONOLOGIA
cv::Mat ImmagineOutput; // per filtro
cv::Mat grabcut_mask_current;
cv::Mat grabcut_mask_undo;
cv::Mat grabcut_mask_redo;
cv::Mat AnteprimaMarcature_current;
cv::Mat AnteprimaMarcature_undo;
cv::Mat AnteprimaMarcature_redo;

// Dimensioni e variabili condivise
int image_width = 0, image_heigth = 0; // dimensioni reali immagine
bool occupato = 0; // se il software sta facendo un compito pesante, le altre azioni non devono essere abilitate

// BoundingBox
int bBox_XStart = 0;
int bBox_YStart = 0;
int bBox_XEnd = 0;
int bBox_YEnd = 0;


