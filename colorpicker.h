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

#ifndef COLORPICKER_H
#define COLORPICKER_H

// Shamelessly copied from https://stackoverflow.com/a/43871405
// Couple of tiny changes by me.

#include <QPushButton>
#include <QColor>

class ColorPicker : public QPushButton
{
    Q_OBJECT
public:
    ColorPicker( QWidget* parent );

    void setColor( const QColor& color );
    const QColor& getColor() const;

public slots:
    void updateColor();
    void changeColor();

signals:
    void colorChanged( const QColor& color );

private:
    QColor color;
};

#endif // COLORPICKER_H
