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

#include "fireeffectwindow.h"
#include "./ui_fireeffectwindow.h"
#include "fireeffectview.h"
#include "colorpicker.h"

#include <QToolTip>

namespace
{
    template< typename _ty, typename _func >
    void SetupSlider( QWidget* source, QSlider* slider, int32_t primer, _ty* widget, _func setfunc )
    {
        slider->setValue( primer );
        slider->setToolTip( QString( "%1" ).arg( primer ) );
        source->connect( slider, &QSlider::valueChanged, widget, setfunc );
        source->connect( slider, &QSlider::sliderMoved, [ slider ]( int32_t val )
        {
            slider->setToolTip( QString("%1").arg( val ) );
            QToolTip::showText( QCursor::pos(), QString( "%1" ).arg( val ), nullptr );
        } );
    }

    template< typename _ty, typename _func >
    void SetupColour( QWidget* source, QWidget* label, ColorPicker* button, int32_t index,
                      QSlider* countslider, QColor primer, _ty* widget, _func setfunc )
    {
        auto enable = [ label, button, index ]( int32_t val )
        {
            bool visible = index <= val;
            label->setVisible( visible );
            button->setVisible( visible );
        };

        button->setColor( primer );
        source->connect( button, &ColorPicker::colorChanged, widget, setfunc );
        source->connect( countslider, &QSlider::valueChanged, enable );

        enable( countslider->value() );
    }
}

FireEffectWindow::FireEffectWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FireEffectWindow)
{
    ui->setupUi(this);

    connect( ui->actionExit, &QAction::triggered, this, &FireEffectWindow::close );

    SetupSlider( this, ui->updateSlider, ui->centralwidget->GetUpdateRate(), ui->centralwidget, &FireEffectView::SetUpdateRate );
    SetupSlider( this, ui->detrateSlider, ui->centralwidget->GetDeteriorationPercent(), ui->centralwidget, &FireEffectView::SetDeteriorationPercent );
    SetupSlider( this, ui->detchanceSlider, ui->centralwidget->GetDeteriorationChance(), ui->centralwidget, &FireEffectView::SetDeteriorationChance );
    SetupSlider( this, ui->windSlider, ui->centralwidget->GetWindStrength(), ui->centralwidget, &FireEffectView::SetWindStrength );
    SetupSlider( this, ui->windcycSlider, ui->centralwidget->GetWindCycleBase(), ui->centralwidget, &FireEffectView::SetWindCycleBase );
    SetupSlider( this, ui->windcurveSlider, ui->centralwidget->GetWindCurvePow(), ui->centralwidget, &FireEffectView::SetWindCurvePow );
    SetupSlider( this, ui->numcolSlider, ui->centralwidget->GetNumFireColours(), ui->centralwidget, &FireEffectView::SetNumFireColours );

    SetupColour( this, ui->col1Label, ui->col1Picker, 1, ui->numcolSlider, ui->centralwidget->GetFireColour< 1 >(), ui->centralwidget, &FireEffectView::SetFireColour< 1 > );
    SetupColour( this, ui->col2Label, ui->col2Picker, 2, ui->numcolSlider, ui->centralwidget->GetFireColour< 2 >(), ui->centralwidget, &FireEffectView::SetFireColour< 2 > );
    SetupColour( this, ui->col3Label, ui->col3Picker, 3, ui->numcolSlider, ui->centralwidget->GetFireColour< 3 >(), ui->centralwidget, &FireEffectView::SetFireColour< 3 > );
    SetupColour( this, ui->col4Label, ui->col4Picker, 4, ui->numcolSlider, ui->centralwidget->GetFireColour< 4 >(), ui->centralwidget, &FireEffectView::SetFireColour< 4 > );
    SetupColour( this, ui->col5Label, ui->col5Picker, 5, ui->numcolSlider, ui->centralwidget->GetFireColour< 5 >(), ui->centralwidget, &FireEffectView::SetFireColour< 5 > );
    SetupColour( this, ui->col6Label, ui->col6Picker, 6, ui->numcolSlider, ui->centralwidget->GetFireColour< 6 >(), ui->centralwidget, &FireEffectView::SetFireColour< 6 > );
    SetupColour( this, ui->col7Label, ui->col7Picker, 7, ui->numcolSlider, ui->centralwidget->GetFireColour< 7 >(), ui->centralwidget, &FireEffectView::SetFireColour< 7 > );
    SetupColour( this, ui->col8Label, ui->col8Picker, 8, ui->numcolSlider, ui->centralwidget->GetFireColour< 8 >(), ui->centralwidget, &FireEffectView::SetFireColour< 8 > );
    SetupColour( this, ui->col9Label, ui->col9Picker, 9, ui->numcolSlider, ui->centralwidget->GetFireColour< 9 >(), ui->centralwidget, &FireEffectView::SetFireColour< 9 > );
}

FireEffectWindow::~FireEffectWindow()
{
    delete ui;
}

