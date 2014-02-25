/**
 * @author Drew Noakes http://drewnoakes.com
 */

import color = require('color');

export interface IHsvColor
{
    hue: number[];
    sat: number[];
    val: number[];
}

export function calculateColour(val: IHsvColor)
{
    var hAvg = (val.hue[0] + val.hue[1]) / 2,
        h = val.hue[0] < val.hue[1] ? hAvg : (hAvg + (255/2)) % 255,
        s = (val.sat[0] + val.sat[1]) / 2,
        v = (val.val[0] + val.val[1]) / 2;

    return new color.Hsv(h/255, s/255, v/255).toString();
}
