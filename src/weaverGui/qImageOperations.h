#ifndef QIMAGEOPERATIONS_H
#define QIMAGEOPERATIONS_H

#include <QImage>
#include "math.h"

template<class T> const T& kClamp( const T& x, const T& low, const T& high );
int changeBrightness( int value, int brightness );
int changeContrast( int value, int contrast );
int changeGamma( int value, int gamma );
int changeUsingTable( int value, const int table[] );
template< int operation( int, int ) > static QImage changeImage( const QImage& image, int value );

QImage changeBrightness( const QImage& image, int brightness );
QImage changeContrast( const QImage& image, int contrast );
QImage changeGamma( const QImage& image, int gamma );


#endif // QIMAGEOPERATIONS_H
