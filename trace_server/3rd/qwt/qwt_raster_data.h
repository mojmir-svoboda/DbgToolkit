/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_RASTER_DATA_H
#define QWT_RASTER_DATA_H 1

#include "qwt_global.h"
#include "qwt_interval.h"
#include <qmap.h>
#include <qlist.h>
#include <qpolygon.h>

class QwtScaleMap;

/*!
  \brief QwtRasterData defines an interface to any type of raster data.

  QwtRasterData is an abstract interface, that is used by
  QwtPlotRasterItem to find the values at the pixels of its raster.

  Gaps inside the bounding rectangle of the data can be indicated by NaN
  values ( when WithoutGaps is disabled ).

  Often a raster item is used to display values from a matrix. Then the
  derived raster data class needs to implement some sort of resampling,
  that maps the raster of the matrix into the requested raster of
  the raster item ( depending on resolution and scales of the canvas ).

  QwtMatrixRasterData implements raster data, that returns values from
  a given 2D matrix.

  \sa QwtMatrixRasterData
*/
class QWT_EXPORT QwtRasterData
{
public:
    //! Contour lines
    typedef QMap<double, QPolygonF> ContourLines;

    /*!
      \brief Raster data attributes

      Additional information that is used to improve processing
      of the data.
    */
    enum Attribute
    {
        /*!
           The bounding rectangle of the data is spanned by
           the interval(Qt::XAxis) and interval(Qt::YAxis)
           WithoutGaps indicates, that the data has no gaps
           ( unknown values ) in this area and the result of
           value() does not need to be checked for NaN values.

           Enabling this flag will have an positive effect on 
           the performance of rendering a QwtPlotSpectrogram.

           The default setting is false.

           \note NaN values indicate an undefined value
         */
        WithoutGaps = 0x01
    };

    //! Raster data Attributes
    typedef QFlags<Attribute> Attributes;

    //! Flags to modify the contour algorithm
    enum ConrecFlag
    {
        //! Ignore all vertices on the same level
        IgnoreAllVerticesOnLevel = 0x01,

        //! Ignore all values, that are out of range
        IgnoreOutOfRange = 0x02
    };

    //! Flags to modify the contour algorithm
    typedef QFlags<ConrecFlag> ConrecFlags;

    QwtRasterData();
    virtual ~QwtRasterData();

    void setAttribute( Attribute, bool on = true );
    bool testAttribute( Attribute ) const;

    virtual void setInterval( Qt::Axis, const QwtInterval & );
    const QwtInterval &interval(Qt::Axis) const;

    virtual QRectF pixelHint( const QRectF & ) const;

    virtual void initRaster( const QRectF &, const QSize& raster );
    virtual void discardRaster();

    /*!
       \return the value at a raster position
       \param x X value in plot coordinates
       \param y Y value in plot coordinates
    */
    virtual double value( double x, double y ) const = 0;

    virtual ContourLines contourLines( const QRectF &rect,
        const QSize &raster, const QList<double> &levels,
        ConrecFlags ) const;

    class Contour3DPoint;
    class ContourPlane;

private:
    // Disabled copy constructor and operator=
    QwtRasterData( const QwtRasterData & );
    QwtRasterData &operator=( const QwtRasterData & );

    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtRasterData::ConrecFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtRasterData::Attributes )

#endif
