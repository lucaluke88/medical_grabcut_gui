#ifndef ESTENSIONI_H
#define ESTENSIONI_H

#include "globals.h"
#include "cvmatqimage.h"

// IMPLEMENTAZIONE DI GROWCUT
static void growCut(cv::Mat& src_image, cv::Mat& grabcut_mask, cv::Mat& dst ){

    cv::Mat label( grabcut_mask.size(), CV_64F );
    for( int y=0; y<grabcut_mask.rows; y++ ){
        for(int x=0;x<grabcut_mask.cols;x++)
        {
            if(grabcut_mask.at<uchar>(y,x) == cv::GC_BGD)
                 label.at<double>(y,x) = 1.0;
            else if (grabcut_mask.at<uchar>(y,x) == cv::GC_FGD)
                label.at<double>(y,x) = -1.0;
            else label.at<double>(y,x) = 0.0;

        }
    }

    cv::Mat strength(grabcut_mask.size(), CV_64F );
    //strength.setTo( Scalar(0) );

    for( int i=0; i<strength.rows; i++ ){
        for( int h=0; h<strength.cols; h++ ){
            double label_val = label.at<double>(i,h);
            if( -0.5 < label_val && label_val < 0.5 ){
                strength.at<double>( i, h ) = 0.0;
            }
            else strength.at<double>( i, h ) = 1.0;
        }
    }

    cv::Mat next_strength = strength.clone();
    cv::Mat next_label = label.clone();

    dst.create( src_image.size(), CV_8U);
    int converged = 0;
    int itr_count = 0;
    do{

        itr_count++;
        if( itr_count % 10 == 0 )
        {
            for( int i=0; i<label.rows; i++ ){
                unsigned char* dst_line = dst.ptr(i);
                for( int h=0; h<label.cols; h++ ){
                    if( label.at<double>(i,h) < -0.5 ) dst_line[h] = cv::GC_BGD;
                    else if( label.at<double>(i,h) < 0.5 ) dst_line[h] = cv::GC_PR_FGD;
                    else dst_line[h] = cv::GC_FGD;
                }
            }

            std::cout<< itr_count << " : " << converged << std::endl;
        }

        converged = 0;

        for( int i=0; i<label.rows; i++ ){
            unsigned char* cline = (unsigned char*)src_image.ptr(i);
            double* cline_label = (double*)label.ptr(i);
            double* cline_strength = (double*)strength.ptr(i);

            double* cline_next_label = (double*)next_label.ptr(i);
            double* cline_next_strength = (double*)next_strength.ptr(i);

            for( int h=0; h<label.cols; h++ ){
                unsigned char* cpix = cline + 3 * h;
                double* clabel = cline_label + h;
                double* cstrength = cline_strength + h;

                double* cnext_label = cline_next_label + h;
                double* cnext_strength = cline_next_strength + h;

                for( int neigh_y=-1; neigh_y<=1; neigh_y++ ){
                    if( i+neigh_y < 0 ||  label.rows <= i+neigh_y) continue;

                    unsigned char* nline = (unsigned char*)src_image.ptr(i+neigh_y);
                    double* nline_label = (double*)label.ptr(i+neigh_y);
                    double* nline_strength = (double*)strength.ptr(i+neigh_y);

                    for( int neigh_x=-1; neigh_x<=1; neigh_x++ ){

                        if( neigh_x == 0 && neigh_y == 0
                            || h+neigh_x < 0 || label.cols <= h+neigh_x) continue;

                        unsigned char* npix = nline + 3 * (h+neigh_x);
                        double* nlabel = nline_label + h+neigh_x;
                        double* nstrength = nline_strength + h+neigh_x;

                        double b = (double)npix[0] - cpix[0];
                        double g = (double)npix[1] - cpix[1];
                        double r = (double)npix[2] - cpix[2];

                        double C = b*b + g*g + r*r;
                        double G = 1 - C / (3 * 255 * 255);

                        /*
                        if( G * strength.at<double>( i+neigh_y, h+neigh_x ) <= strength.at<double>( i, h )) continue;

                            converged++;

                        next_label.at<double>( i, h ) = label.at<double>( i+neigh_y, h+neigh_x );
                        next_strength.at<double>( i, h ) = G * strength.at<double>( i+neigh_y, h+neigh_x );
                        /*/
                        if( G * *nstrength <= *cstrength ) continue;

                        converged++;

                        //next_label.at<double>(i,h) = *nlabel;
                        //next_strength.at<double>(i,h) = G * *nstrength;
                        *cnext_label = *nlabel;
                        *cnext_strength = G * *nstrength;
                        //*/

                    }
                }
            }
        }

        next_label.copyTo( label );
        next_strength.copyTo( strength );

    } while( converged > 0 );


}



#endif // ESTENSIONI_H
