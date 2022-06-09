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

#include "colorpicker.h"
#include <QColorDialog>

ColorPicker::ColorPicker( QWidget* parent )
    : QPushButton(parent)
{
    connect( this, SIGNAL(clicked()), this, SLOT(changeColor()) );
}

void ColorPicker::updateColor()
{
    setStyleSheet( "background-color: " + color.name() );
}

void ColorPicker::changeColor()
{
    QColor newColor = QColorDialog::getColor( color, parentWidget(), "Choose a colour", QColorDialog::DontUseNativeDialog );
    if ( newColor != color )
    {
        setColor( newColor );
    }
}

void ColorPicker::setColor( const QColor& color )
{
    this->color = color;
    updateColor();

    emit colorChanged( color );
}

const QColor& ColorPicker::getColor() const
{
    return color;
}
