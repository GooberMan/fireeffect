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

#include "fireeffectview.h"

#include <QOpenGLFunctions>
#include <QRandomGenerator>

namespace
{
    struct FireEffectVertex
    {
        QVector3D   pos;
        QVector2D   uv;

        static constexpr size_t PosOffset() noexcept;
        static constexpr size_t UVOffset() noexcept;
    };

    constexpr size_t FireEffectVertex::PosOffset() noexcept { return offsetof( FireEffectVertex, pos ); }
    constexpr size_t FireEffectVertex::UVOffset() noexcept { return offsetof( FireEffectVertex, uv ); }

    // Brace initialisers for discrete elements do not work in MSVC
    // on std::array constexpr when using structs, so for readability
    // we have a MakeVec function
    template< size_t num > struct vecselect { };
    template<> struct vecselect< 2 > { using type = QVector2D; };
    template<> struct vecselect< 3 > { using type = QVector3D; };
    template<> struct vecselect< 4 > { using type = QVector4D; };
    template< size_t num > using vecselect_t = typename vecselect< num >::type;

    template< typename... _args >
    static consteval auto MakeVec( _args... args )
    {
        return vecselect_t< sizeof...( args ) >( args... );
    }

    // Going projectionless, these vertices map to clip range already for full screen coverage
    static constexpr std::array< FireEffectVertex, 4 > BasicQuadVertices =
    {
        MakeVec( -1.0f, -1.0f, 0.0f ), MakeVec( 0.0f, 1.0f ),
        MakeVec( -1.0f,  1.0f, 0.0f ), MakeVec( 0.0f, 0.0f ),
        MakeVec(  1.0f, -1.0f, 0.0f ), MakeVec( 1.0f, 1.0f ),
        MakeVec(  1.0f,  1.0f, 0.0f ), MakeVec( 1.0f, 0.0f ),
    };

    static constexpr std::array< QVector3D, FireEffectView::MaxFireColours > DefaultFireColours =
    {
        MakeVec( 0.0f,      0.0f,       0.0f    ),
        MakeVec( 0.624f,    0.184f,     0.027f  ),
        MakeVec( 0.812f,    0.435f,     0.059f  ),
        MakeVec( 0.749f,    0.655f,     0.153f  ),
        MakeVec( 0.812f,    0.812f,     0.435f  ),
        MakeVec( 0.0f,      0.0f,       0.0f    ),
        MakeVec( 0.0f,      0.0f,       0.0f    ),
        MakeVec( 0.0f,      0.0f,       0.0f    ),
        MakeVec( 0.0f,      0.0f,       0.0f    ),
        MakeVec( 0.0f,      0.0f,       0.0f    ),
    };

    static constexpr int32_t DefaultNumFireColours = 5;
}

//=============================================================================
FireEffectView::FireEffectView( QWidget *parent )
    : Super( parent )
    , _uniformAccumulatorSource( 0xFFFFFFFF )
    , _uniformDeteriorationPercent( 0xFFFFFFFF )
    , _uniformDeteriorationChance( 0xFFFFFFFF )
    , _uniformWindStrength( 0xFFFFFFFF )
    , _uniformAppTime( 0xFFFFFFFF )
    , _uniformAccumulatorStart( 0xFFFFFFFF )
    , _uniformAccumulatorEnd( 0xFFFFFFFF )
    , _uniformAccumulatorLerp( 0xFFFFFFFF )
    , _uniformEffectColours( 0xFFFFFFFF )
    , _uniformNumEffectColours( 0xFFFFFFFF )
    , _frameTimer( new QTimer( parent ) )
    , _timeAccumulator( 0 )
    , _totalTime( 0 )
    , _framesPerSecond( DefaultFramesPerSecond )
    , _deteriorationPercent( DefaultDeteriorationPercent )
    , _deteriorationChance( DefaultDeteriorationChance )
    , _windStrength( DefaultWindStrength )
    , _windCycleBase( DefaultWindCycleBase )
    , _windCurvePow( DefaultWindCurvePow )
    , _fireColours( DefaultFireColours )
    , _numFireColours( DefaultNumFireColours )
{
    _frameTimer->setInterval( 0 );
    connect( _frameTimer.get(), SIGNAL( timeout() ), this, SLOT( OnFrame() ) );
}

//=============================================================================
FireEffectView::~FireEffectView()
{
}

//=============================================================================
void FireEffectView::initializeGL()
{
    Super::initializeGL();

    _accumulationBufferSource = CreateAccumulationBuffer( DefaultBufferWidth, DefaultBufferHeight );
    _accumulationBufferTarget = CreateAccumulationBuffer( DefaultBufferWidth, DefaultBufferHeight );

    _vertexBuffer = std::unique_ptr< QOpenGLBuffer >( new QOpenGLBuffer );
    _vertexBuffer->create();
    _vertexBuffer->bind();
    _vertexBuffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
    _vertexBuffer->allocate( BasicQuadVertices.data(), BasicQuadVertices.size() * sizeof( FireEffectVertex ) );

    _vertexObject = std::unique_ptr< QOpenGLVertexArrayObject >( new QOpenGLVertexArrayObject );
    _vertexObject->create();
    _vertexObject->bind();

    _accumulationProgram = CreateProgram( ":shaders/basic.vsh", ":shaders/fireaccumulation.fsh" );
    _uniformAccumulatorSource       = _accumulationProgram->uniformLocation( "accumulatorsource" );
    _uniformDeteriorationPercent    = _accumulationProgram->uniformLocation( "deteriorationpercent" );
    _uniformDeteriorationChance     = _accumulationProgram->uniformLocation( "deteriorationchance" );
    _uniformWindStrength            = _accumulationProgram->uniformLocation( "windstrength" );
    _uniformWindCycleBase           = _accumulationProgram->uniformLocation( "windcyclebase" );
    _uniformWindCurvePow            = _accumulationProgram->uniformLocation( "windcurvepow" );
    _uniformAppTime                 = _accumulationProgram->uniformLocation( "apptime" );

    _finalRenderProgram             = CreateProgram( ":shaders/basic.vsh", ":shaders/firerender.fsh" );
    _uniformAccumulatorStart        = _finalRenderProgram->uniformLocation( "accumulatorstart" );
    _uniformAccumulatorEnd          = _finalRenderProgram->uniformLocation( "accumulatorend" );
    _uniformAccumulatorLerp         = _finalRenderProgram->uniformLocation( "accumulatorlerp" );
    _uniformEffectColours           = _finalRenderProgram->uniformLocation( "effectcolours" );
    _uniformNumEffectColours        = _finalRenderProgram->uniformLocation( "numeffectcolours" );

    _vertexObject->release();
    _vertexBuffer->release();

    QOpenGLFunctions *GL = QOpenGLContext::currentContext()->functions();

    GL->glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    GL->glClearColor( 0.1f, 0.1f, 0.3f, 1.0f );

    GL->glActiveTexture( GL_TEXTURE0 );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    GL->glActiveTexture( GL_TEXTURE0 + 1 );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    _frameTimer->start();
    _baseTime = std::chrono::system_clock::now();
}

//=============================================================================
void FireEffectView::OnFrame()
{
    // Results will be lerped, so we want to render as fast as we can
    update();
}

//=============================================================================
void FireEffectView::CheckSimulation()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::duration< double_t > delta = now - _baseTime;
    _baseTime = now;

    _timeAccumulator += delta.count();
    _totalTime += delta.count();
    double_t timestep = 1.0 / _framesPerSecond;

    while( _timeAccumulator >= timestep )
    {
        UpdateSimulation();
        _timeAccumulator -= timestep;
    }

}

//=============================================================================
void FireEffectView::UpdateSimulation()
{
    QOpenGLFunctions *GL = QOpenGLContext::currentContext()->functions();
    _accumulationBufferTarget->bind();
    GL->glViewport( 0, 0, DefaultBufferWidth, DefaultBufferHeight );

    GL->glActiveTexture( GL_TEXTURE0 );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    GL->glBindTexture( GL_TEXTURE_2D, _accumulationBufferSource->texture() );

    _accumulationProgram->bind();
    _accumulationProgram->setUniformValue( _uniformAccumulatorSource, 0 );
    _accumulationProgram->setUniformValue( _uniformDeteriorationPercent, (GLfloat)_deteriorationPercent );
    _accumulationProgram->setUniformValue( _uniformDeteriorationChance, (GLfloat)_deteriorationChance );
    _accumulationProgram->setUniformValue( _uniformWindStrength, (GLfloat)_windStrength );
    _accumulationProgram->setUniformValue( _uniformWindCycleBase, (GLfloat)_windCycleBase );
    _accumulationProgram->setUniformValue( _uniformWindCurvePow, (GLfloat)_windCurvePow );
    _accumulationProgram->setUniformValue( _uniformAppTime, (GLfloat)_totalTime );

    _vertexObject->bind();
    GL->glDrawArrays( GL_TRIANGLE_STRIP, 0, (int32_t)BasicQuadVertices.size() );
    _vertexObject->release();

    _accumulationProgram->release();
    _accumulationBufferTarget->release();
    GL->glBindTexture( GL_TEXTURE_2D, 0 );
    std::swap( _accumulationBufferSource, _accumulationBufferTarget );
}

//=============================================================================
void FireEffectView::resizeGL( int32_t w, int32_t h )
{
    Super::resizeGL( w, h );
}

//=============================================================================
void FireEffectView::paintGL()
{
    Super::paintGL();

    CheckSimulation();

    double_t timestep = 1.0 / _framesPerSecond;
    double_t lerppercent = _timeAccumulator / timestep;

    QOpenGLFunctions *GL = QOpenGLContext::currentContext()->functions();

    GL->glActiveTexture( GL_TEXTURE0 );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    GL->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    GL->glBindTexture( GL_TEXTURE_2D, _accumulationBufferTarget->texture() );

    GL->glActiveTexture( GL_TEXTURE0 + 1 );
    GL->glBindTexture( GL_TEXTURE_2D, _accumulationBufferSource->texture() );

    GL->glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    GL->glViewport( 0, 0, width(), height() );
    GL->glClearColor( 0.1f, 0.1f, 0.25f, 1.0f );

    _finalRenderProgram->bind();
    _finalRenderProgram->setUniformValue( _uniformAccumulatorStart, 0 );
    _finalRenderProgram->setUniformValue( _uniformAccumulatorEnd, 1 );
    _finalRenderProgram->setUniformValue( _uniformAccumulatorLerp, (GLfloat)lerppercent );
    _finalRenderProgram->setUniformValueArray( _uniformEffectColours, _fireColours.data(), (GLint)_fireColours.size() );
    _finalRenderProgram->setUniformValue( _uniformNumEffectColours, _numFireColours );

    _vertexObject->bind();
    GL->glDrawArrays( GL_TRIANGLE_STRIP, 0, (int32_t)BasicQuadVertices.size() );
    _vertexObject->release();
}

//=============================================================================
std::unique_ptr< QOpenGLFramebufferObject > FireEffectView::CreateAccumulationBuffer( int32_t width, int32_t height )
{
    QOpenGLFramebufferObjectFormat format;
    format.setMipmap( false );
    format.setTextureTarget( GL_TEXTURE_2D );
    // Not at all clear from the documentation, had to dig through the source.
    // 0 samples results in creating a render texture instead of a colour attachment.
    format.setSamples( 0 );
    format.setInternalTextureFormat( GL_R16F );
    format.setAttachment( QOpenGLFramebufferObject::NoAttachment );

    std::unique_ptr< QOpenGLFramebufferObject > buffer( new QOpenGLFramebufferObject( width, height, format ) );
    return buffer;
}

//=============================================================================
std::unique_ptr< QOpenGLShaderProgram > FireEffectView::CreateProgram( const char* vertex, const char* fragment )
{
    std::unique_ptr< QOpenGLShaderProgram > program( new QOpenGLShaderProgram( this ) );

    program->addShaderFromSourceFile( QOpenGLShader::Vertex, vertex );
    program->addShaderFromSourceFile( QOpenGLShader::Fragment, fragment );
    program->link();
    program->bind();

    int32_t posLocation = program->attributeLocation( "position" );
    int32_t uvLocation = program->attributeLocation( "uv" );

    program->enableAttributeArray( posLocation );
    program->setAttributeBuffer( posLocation, GL_FLOAT, FireEffectVertex::PosOffset(), 3, (int32_t)sizeof( FireEffectVertex ) );
    program->enableAttributeArray( uvLocation );
    program->setAttributeBuffer( uvLocation, GL_FLOAT, FireEffectVertex::UVOffset(), 2, (int32_t)sizeof( FireEffectVertex ) );

    program->release();

    return program;
}
