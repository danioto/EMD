////////////////////////////////////////////////////////////////////////////////
//              Empirical Mode Decomposition                                  //
// BERNARD Guillaume                                                          //
// DURAND William                                                             //
// ZZ3F2 ISIMA                                                                //
////////////////////////////////////////////////////////////////////////////////

#include "CImg.h"
#include <math.h>
#include <vector>
#include <iostream>

#include "Euclidean.hpp"

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

using namespace cimg_library;

double Sum(CImg<unsigned char> img, int startedX, int startedY, int w) {
    double res = 0;
    for(int i = startedX - ((w - 1) / 2); i < startedX + ((w + 1) / 2); i++) {
        for(int j = startedY - ((w - 1) / 2) ; j < startedY + ((w + 1) / 2); j++) {
            if( (i >= 0 && i < img.width()) && (j >= 0  && j < img.height()) ) {
                res += img(i,j);
            }
        }
    }
    return res;
}

void ShowMatrix(CImg<unsigned char> img) {
    std::cout << std::endl;
    for (int i = 0; i < 9 ; i++) {
        for (int j = 0; j < 9; j++) {
            std::cout << (double) img(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/*******************************************************************************
  Main
 *******************************************************************************/
int main()
{
#ifdef DEBUG
    CImg<unsigned char> inputImg(8, 8, 1, 3);
    int tab[][8] = {
        { 8, 8, 4, 1, 5, 2, 6, 3 },
        { 6, 3, 2, 3, 7, 3, 9, 3 },
        { 7, 8, 3, 2, 1, 4, 3, 7 },
        { 4, 1, 2, 4, 3, 5, 7, 8 },
        { 6, 4, 2, 1, 2, 5, 3, 4 },
        { 1, 3, 7, 9, 9, 8, 7, 8 },
        { 9, 2, 6, 7, 6, 8, 7, 7 },
        { 8, 2, 1, 9, 7, 9, 1, 1 }
    };

    printf("Base:\n");
    for (int i = 0; i < inputImg.width(); i++) {
        for (int j = 0; j < inputImg.height(); j++) {
            inputImg(i, j) = tab[i][j];
            printf("%d ", inputImg(i, j));
        }
        printf("\n");
    }
#else
    CImg<unsigned char> inputImg("lena.bmp");
    CImgDisplay dispBase(inputImg,"Image de base");
#endif

    std::vector<Euclidean> vectEMax, vectEMin;

    ///////////////////////////////////////////////////////////////////////////////
    //              Part 1: Finding minimas and maximas                          //
    ///////////////////////////////////////////////////////////////////////////////

    CImg<unsigned char> imgMax(inputImg.channel(0));
    CImg<unsigned char> imgMin(inputImg.channel(0));

    for (int i = 0; i < inputImg.width(); i += 3) {
        for (int j = 0; j < inputImg.height(); j += 3) {

            // Save max and min locations
            int xmax = i;
            int ymax = j;
            int xmin = i;
            int ymin = j;

            // save values
            unsigned char max = imgMax(i,j);
            unsigned char min = imgMin(i,j);

            Euclidean eMax(i, j);
            Euclidean eMin(i, j);

            // 3x3
            for (int k = i; k<i+3 ; k++) {
                for (int l = j; l<j+3 ; l++) {

                    // Max
                    if ((imgMax(k,l) <= max) && (l!=ymax || k!=xmax)) {
                        imgMax(k,l) = 0;
                    } else if (l!=ymax || k!=xmax) {
                        max = imgMax(k,l);
                        imgMax(xmax,ymax) = 0;
                        xmax = k;
                        ymax = l;

                        eMax.setX(k);
                        eMax.setY(l);
                    }

                    // Min
                    if ((imgMin(k,l) >= min) && (l!=ymin || k!=xmin)) {
                        imgMin(k,l) = 0;
                    } else if (l!=ymax || k!=xmax) {
                        min = imgMin(k,l);
                        imgMin(xmin,ymin) = 0;
                        xmin = k;
                        ymin = l;

                        eMin.setX(k);
                        eMin.setY(l);
                    }
                }
            }

            vectEMax.push_back(eMax);
            vectEMin.push_back(eMin);
        }
    }

#ifdef DEBUG
    printf("Extremas:\n");

    printf("Max\n");
    for (int i = 0; i < imgMax.width(); i++) {
        for (int j = 0; j < imgMax.height(); j++) {
            printf("%d ", imgMax(i, j));
        }
        printf("\n");
    }

    printf("Min\n");
    for (int i = 0; i < imgMin.width(); i++) {
        for (int j = 0; j < imgMin.height(); j++) {
            printf("%d ", imgMin(i, j));
        }
        printf("\n");
    }
#endif

    // Array of Euclidean distance to the nearest non zero element
    std::vector<Euclidean>::iterator it1, it2;

    for (it1 = vectEMax.begin(); it1 != vectEMax.end(); it1++) {
        for (it2 = it1 + 1; it2 != vectEMax.end(); it2++) {
            double dist = (*it1).computeDistanceFrom(*it2);

            if (0 == (*it1).getDistance() || dist < (*it1).getDistance()) {
                (*it1).setDistance(dist);
                (*it1).setNearest(*it2);
            }

            if (0 == (*it2).getDistance() || dist < (*it2).getDistance()) {
                (*it2).setDistance(dist);
                (*it2).setNearest(*it1);
            }
        }
    }

    for (it1 = vectEMin.begin(); it1 != vectEMin.end(); it1++) {
        for (it2 = it1 + 1; it2 != vectEMin.end(); it2++) {
            double dist = (*it1).computeDistanceFrom(*it2);

            if (0 == (*it1).getDistance() || dist < (*it1).getDistance()) {
                (*it1).setDistance(dist);
                (*it1).setNearest(*it2);
            }

            if (0 == (*it2).getDistance() || dist < (*it2).getDistance()) {
                (*it2).setDistance(dist);
                (*it2).setNearest(*it1);
            }
        }
    }

    int wmax = 0;
    std::vector<int> w;

    // Calculate the windows sizes
    for(unsigned int i = 0; i < vectEMin.size(); i++) {
        double d1 = MIN(vectEMax[i].getDistance(), vectEMin[i].getDistance());
        double d2 = MAX(vectEMax[i].getDistance(), vectEMin[i].getDistance());
        double d3 = MIN(vectEMax[i].getDistance(), vectEMin[i].getDistance());
        double d4 = MAX(vectEMax[i].getDistance(), vectEMin[i].getDistance());

        int wi = (int)ceil(MIN(MIN(d1, d2), MIN(d3, d4)));
        if(wi%2 == 0)
        {
            wi++;
        }
        if(wi > wmax) {
            wmax=wi;
        }
        w.push_back(wi);
    }

    CImg<unsigned char> imgSource(inputImg.channel(0));

    // Order filters with source image
    std::vector<unsigned char> vectFilterMax, vectFilterMin;

    for(int unsigned i = 0; i < vectEMax.size(); i++) {
        unsigned char max = 0;
        for (int k = vectEMax[i].getX() - ((w[i] - 1) / 2); k < vectEMax[i].getX() + ((w[i] + 1) / 2); k++) {
            for (int l = vectEMax[i].getY() - ((w[i] - 1) / 2); l < vectEMax[i].getY() + ((w[i] + 1) / 2); l++) {
                if( (k>=0 && k<imgSource.width()) && (l>=0  && l<imgSource.height()) ) {
                    if (imgSource(k, l) > max) {
                        max = imgSource(k, l);
                    }
                }
            }
        }
        vectFilterMax.push_back(max);
    }

    for(int unsigned i = 0; i < vectEMin.size(); i++) {
        unsigned char min = 255;
        for (int k = vectEMin[i].getX() - ((w[i] - 1) / 2); k < vectEMin[i].getX() + ((w[i] + 1) / 2); k++) {
            for (int l = vectEMin[i].getY() - ((w[i] - 1) / 2); l < vectEMin[i].getY() + ((w[i] + 1) / 2); l++) {
                if( (k>=0 && k<imgSource.width()) && (l>=0  && l<imgSource.height()) ) {
                    if (imgSource(k, l) < min) {
                        min = imgSource(k, l);
                    }
                }
            }
        }
        vectFilterMin.push_back(min);
    }

    CImg<unsigned char> newImgMax(imgMax.width(), imgMax.height());

    // Calculate the upper envelope
    for(int unsigned i = 0; i < vectEMax.size(); i++) {
        for (int k = vectEMax[i].getX() - ((w[i] - 1) / 2); k < vectEMax[i].getX() + ((w[i] + 1) / 2); k++) {
            for (int l = vectEMax[i].getY() - ((w[i] - 1) / 2); l < vectEMax[i].getY() + ((w[i] + 1) / 2); l++) {
                if( (k>=0 && k<imgSource.width()) && (l>=0  && l<imgSource.height()) ) {
                    imgMax(k, l) = vectFilterMax[i];
                }
            }
        }
    }

#ifdef DEBUG
    printf("Envelopes:\n");

    printf("Max\n");
    for (int i = 0; i < newImgMax.width(); i++) {
        for (int j = 0; j < newImgMax.height(); j++) {
            printf("%d ", newImgMax(i, j));
        }
        printf("\n");
    }
#endif

    // Smooth of the upper envelope
    for (int k = 0; k < imgSource.width(); k++) {
        for (int l = 0; l < imgSource.height(); l++) {
            if( (k>=0 && k<imgSource.width()) && (l>=0  && l<imgSource.height()) ) {
                newImgMax(k, l) = (int)Sum(imgMax, k, l,wmax)/(wmax*wmax);
            }
        }
    }

    CImg<unsigned char> newImgMin(imgMin.width(), imgMin.height());

    // Calculate the lower envelope
    for(int unsigned i = 0; i < vectEMin.size(); i++) {
        for (int k = vectEMin[i].getX() - ((w[i] - 1) / 2); k < vectEMin[i].getX() + ((w[i] + 1) / 2); k++) {
            for (int l = vectEMin[i].getY() - ((w[i] - 1) / 2); l < vectEMin[i].getY() + ((w[i] + 1) / 2); l++) {
                if( (k>=0 && k<imgSource.width()) && (l>=0  && l<imgSource.height()) ) {
                    imgMin(k, l) = vectFilterMin[i];
                }
            }
        }
    }

#ifdef DEBUG
    printf("Min\n");
    for (int i = 0; i < newImgMin.width(); i++) {
        for (int j = 0; j < newImgMin.height(); j++) {
            printf("%d ", newImgMin(i, j));
        }
        printf("\n");
    }
#endif

    // Smooth of the lower envelope
    for (int k = 0; k < imgSource.width(); k++) {
        for (int l = 0; l < imgSource.height(); l++) {
            if( (k>=0 && k<imgSource.width()) && (l>=0  && l<imgSource.height()) ) {
                newImgMin(k, l) = (int)Sum(imgMin, k, l,wmax)/(wmax*wmax);
            }
        }
    }

#ifdef DEBUG
    printf("Smoothed envelopes:\n");

    printf("Max\n");
    for (int i = 0; i < newImgMax.width(); i++) {
        for (int j = 0; j < newImgMax.height(); j++) {
            printf("%d ", newImgMax(i, j));
        }
        printf("\n");
    }

    printf("Min\n");
    for (int i = 0; i < newImgMin.width(); i++) {
        for (int j = 0; j < newImgMin.height(); j++) {
            printf("%d ", newImgMin(i, j));
        }
        printf("\n");
    }
#else
    // Display images for max and min
    CImgDisplay dispMax(imgMax,"Image de Max");
    CImgDisplay dispMin(imgMin,"Image de Min");
    CImgDisplay dispEMax(newImgMax,"Image de enveloppe Max");
    CImgDisplay dispEMin(newImgMin,"Image de enveloppe Min");
#endif

    ///////////////////////////////////////////////////////////////////////////////
    //                       Part 2: Average                                     //
    ///////////////////////////////////////////////////////////////////////////////

    // Calculate the Average
    CImg<unsigned char> imgMoyenne(inputImg.width(), inputImg.height());

    for (int i = 0; i < inputImg.width(); i++) {
        for (int j = 0; j < inputImg.height(); j++) {
            imgMoyenne(i, j) = (newImgMin(i, j) + newImgMax(i, j)) /2;
        }
    }

#ifdef DEBUG
    printf("Average:\n");

    for (int i = 0; i < imgMoyenne.width(); i++) {
        for (int j = 0; j < imgMoyenne.height(); j++) {
            printf("%d ", imgMoyenne(i, j));
        }
        printf("\n");
    }
#else
    CImgDisplay dispMoyenne(imgMoyenne, "Image Moyenne");
#endif

    ///////////////////////////////////////////////////////////////////////////////
    //                         Partie 3: Deletion                                //
    ///////////////////////////////////////////////////////////////////////////////


#ifndef DEBUG
    CImg<unsigned char> imgFin(inputImg - imgMoyenne);
    CImgDisplay dispFin(imgFin, "Image Finale");

    while (!dispBase.is_closed()) {
        dispBase.wait();
    }
#endif

    return 0;
}
