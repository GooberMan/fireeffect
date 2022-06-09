// Fire Effect
// Copyright (C) 2022 Ethan Watson
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FIREEFFECTVIEW_H
#define FIREEFFECTVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QTimer>

#include <memory>
#include <chrono>
#include <array>

class FireEffectView : public QOpenGLWidget
{
    Q_OBJECT

    // These property functions are specifically designed to work with the UI.
    // They do not necessarily reflect the actual state of values within the object.

    // Values are in Hertz
    Q_PROPERTY( int32_t UpdateRate READ GetUpdateRate WRITE SetUpdateRate NOTIFY UpdateRateChanged );

    // Quantized to an integral percentage
    Q_PROPERTY( int32_t DeteriorationPercent READ GetDeteriorationPercent WRITE SetDeteriorationPercent NOTIFY DeteriorationPercentChanged );

    // Quantized to an integral percentage
    Q_PROPERTY( int32_t DeteriorationChance READ GetDeteriorationChance WRITE SetDeteriorationChance NOTIFY DeteriorationChanceChanged );

    // Quantized to an integral percentage
    Q_PROPERTY( int32_t WindStrength READ GetWindStrength WRITE SetWindStrength NOTIFY WindStrengthChanged );

    // Quantized to an integral percentage
    Q_PROPERTY( int32_t WindCycleBase READ GetWindCycleBase WRITE SetWindCycleBase NOTIFY WindCycleBaseChanged );

    // Quantized to an integral percentage
    Q_PROPERTY( int32_t WindCurvePow READ GetWindCurvePow WRITE SetWindCurvePow NOTIFY WindCurvePowChanged );

    // NumFireColours will always report at least 1 colour, as we want the base black colour to never change so we can fade to it nicely
    Q_PROPERTY( int32_t NumFireColours READ GetNumFireColours WRITE SetNumFireColours NOTIFY NumFireColoursChanged );

    using Super = QOpenGLWidget;

public:
    static constexpr int32_t MaxFireColours = 10;
    static constexpr int32_t DefaultFramesPerSecond = 300;
    static constexpr int32_t DefaultBufferHeight = 640;
    static constexpr int32_t DefaultBufferWidth = DefaultBufferHeight * 16 / 9;

    static constexpr double_t DefaultDeteriorationPercent = 0.05;
    static constexpr double_t DefaultDeteriorationChance = 0.2;
    static constexpr double_t DefaultWindStrength = 0;
    static constexpr double_t DefaultWindCycleBase = 0.92;
    static constexpr double_t DefaultWindCurvePow = 1.5;

    FireEffectView(QWidget *parent = nullptr);
    virtual ~FireEffectView();

    int32_t                                         GetUpdateRate()                         { return _framesPerSecond; }
    int32_t                                         GetDeteriorationPercent()               { return _deteriorationPercent * 100; }
    int32_t                                         GetDeteriorationChance()                { return _deteriorationChance * 100; }
    int32_t                                         GetWindStrength()                       { return _windStrength * 100; }
    int32_t                                         GetWindCycleBase()                      { return _windCycleBase * 100; }
    int32_t                                         GetWindCurvePow()                       { return _windCurvePow * 100; }
    int32_t                                         GetNumFireColours()                     { return _numFireColours - 1; }

    // Templated so I can throw around plain function pointers for the correct index to Qt's signals etc
    template <size_t Index >
    QColor                                          GetFireColour()
    {
        return QColor( _fireColours[ Index ].x() * 255, _fireColours[ Index ].y() * 255, _fireColours[ Index ].z() * 255, 1 );
    }

    template <size_t Index >
    void                                            SetFireColour( const QColor& val )
    {
        _fireColours[ Index ] = QVector3D( val.red() / 255.0, val.green() / 255.0, val.blue() / 255.0 );
        // TODO: Handle the emit signal
    }

public slots:
    void                                            OnFrame();

    void                                            SetUpdateRate( int32_t val )            { _framesPerSecond = val; emit UpdateRateChanged( val ); }
    void                                            SetDeteriorationPercent( int32_t val )  { _deteriorationPercent = val * 0.01; emit DeteriorationPercentChanged( val ); }
    void                                            SetDeteriorationChance( int32_t val )   { _deteriorationChance = val * 0.01; emit DeteriorationChanceChanged( val ); }
    void                                            SetWindStrength( int32_t val )          { _windStrength = val * 0.01; emit WindStrengthChanged( val ); }
    void                                            SetWindCycleBase( int32_t val )         { _windCycleBase = val * 0.01; emit WindCycleBaseChanged( val ); }
    void                                            SetWindCurvePow( int32_t val )          { _windCurvePow = val * 0.01; emit WindCurvePowChanged( val ); }
    void                                            SetNumFireColours( int32_t val )        { _numFireColours = val + 1; emit NumFireColoursChanged( val ); }

signals:
    void                                            UpdateRateChanged( int32_t val );
    void                                            DeteriorationPercentChanged( int32_t val );
    void                                            DeteriorationChanceChanged( int32_t val );
    void                                            WindStrengthChanged( int32_t val );
    void                                            WindCycleBaseChanged( int32_t val );
    void                                            WindCurvePowChanged( int32_t val );
    void                                            NumFireColoursChanged( int32_t val );

protected:
    void                                            CheckSimulation();
    void                                            UpdateSimulation();

    virtual void                                    initializeGL() override;
    virtual void                                    resizeGL( int32_t w, int32_t h ) override;
    virtual void                                    paintGL() override;

    std::unique_ptr< QOpenGLFramebufferObject >     CreateAccumulationBuffer( int32_t width, int32_t height );
    std::unique_ptr< QOpenGLShaderProgram >         CreateProgram( const char* vertex, const char* fragment );

private:

    std::unique_ptr< QOpenGLFramebufferObject >     _accumulationBufferSource;
    std::unique_ptr< QOpenGLFramebufferObject >     _accumulationBufferTarget;

    std::unique_ptr< QOpenGLBuffer >                _vertexBuffer;
    std::unique_ptr< QOpenGLVertexArrayObject >     _vertexObject;

    std::unique_ptr< QOpenGLShaderProgram >         _accumulationProgram;
    GLuint                                          _uniformAccumulatorSource;
    GLuint                                          _uniformDeteriorationPercent;
    GLuint                                          _uniformDeteriorationChance;
    GLuint                                          _uniformWindStrength;
    GLuint                                          _uniformWindCycleBase;
    GLuint                                          _uniformWindCurvePow;
    GLuint                                          _uniformAppTime;

    std::unique_ptr< QOpenGLShaderProgram >         _finalRenderProgram;
    GLuint                                          _uniformAccumulatorStart;
    GLuint                                          _uniformAccumulatorEnd;
    GLuint                                          _uniformAccumulatorLerp;
    GLuint                                          _uniformEffectColours;
    GLuint                                          _uniformNumEffectColours;

    std::unique_ptr< QTimer >                       _frameTimer;

    std::chrono::system_clock::time_point           _baseTime;
    double_t                                        _timeAccumulator;
    double_t                                        _totalTime;
    int32_t                                         _framesPerSecond;

    double_t                                        _deteriorationPercent;
    double_t                                        _deteriorationChance;
    double_t                                        _windStrength;
    double_t                                        _windCycleBase;
    double_t                                        _windCurvePow;

    std::array< QVector3D, MaxFireColours >         _fireColours;
    int32_t                                         _numFireColours;
};

#endif // FIREEFFECTVIEW_H
