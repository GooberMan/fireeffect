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

in vec2 curruv;

uniform sampler2D   accumulatorsource;
uniform float       deteriorationpercent;
uniform float       deteriorationchance;
uniform float       windstrength;
uniform float       windcyclebase;
uniform float       windcurvepow;
uniform float       apptime;

layout( location = 0 ) out float accumulatedval;

// Random function from https://thebookofshaders.com/10/
// Think I'll change this to some other kind of seed-based random
// Ends up looking too patterny for my needs
float random( vec2 st )
{
    return fract( sin( dot( st, vec2(12.9898,78.233) ) ) * 43758.5453123 );
}

void main()
{
    vec2 accumulatorsize = vec2( textureSize( accumulatorsource, 0 ) );

    vec2 randomseed = curruv + apptime;
    float randomval = random( randomseed );

    float windcycle = windcyclebase + sin( apptime * 4 ) * ( 1.0 - windcyclebase );
    float windamount = pow( curruv.y, windcurvepow ) * ( accumulatorsize.x / 8.0 + randomval ) * ( windstrength * windcycle );

    float actualdeterioration = randomval < deteriorationchance ? deteriorationpercent : 0.0;

    vec2 thisuv = ( vec2( 1.0 ) - curruv ) + vec2( ( windamount + ( min( randomval * 3, 2 ) - 1 ) ) / accumulatorsize.x, 1.0 / accumulatorsize.y );
    float val = ( thisuv.y * accumulatorsize.y >= accumulatorsize.y - ( 1.0 / accumulatorsize.y ) ) ? 1.0 : texture( accumulatorsource, thisuv ).x;
    float deteriorationamount = 1.0 - actualdeterioration;

    accumulatedval = val * deteriorationamount;
}
