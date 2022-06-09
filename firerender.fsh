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

#version 410

uniform sampler2D   accumulatorstart;
uniform sampler2D   accumulatorend;
uniform float       accumulatorlerp;
uniform vec3        effectcolours[ 10 ];
uniform int         numeffectcolours;

in vec2 curruv;

layout( location = 0 ) out vec4 colour;

void main()
{
    float sampledstart  = texture( accumulatorstart, curruv ).x;
    float sampledend    = texture( accumulatorend, curruv ).x;
    float final         = mix( sampledstart, sampledend, accumulatorlerp );

    float lookup        = float( numeffectcolours ) * final;
    int startind        = int( min( floor( lookup ), numeffectcolours - 1 ) );
    int endind          = int( min( startind + 1, numeffectcolours - 1 ) );
    float percentage    = fract( lookup );

    colour = vec4( mix( effectcolours[ startind ], effectcolours[ endind ], percentage ), 1.0 );
}
