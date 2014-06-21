/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts"/>

//noinspection SpellCheckingInspection
var knownColors = {
    aliceblue: 'f0f8ff',
    antiquewhite: 'faebd7',
    aqua: '00ffff',
    aquamarine: '7fffd4',
    azure: 'f0ffff',
    beige: 'f5f5dc',
    bisque: 'ffe4c4',
    black: '000000',
    blanchedalmond: 'ffebcd',
    blue: '0000ff',
    blueviolet: '8a2be2',
    brown: 'a52a2a',
    burlywood: 'deb887',
    cadetblue: '5f9ea0',
    chartreuse: '7fff00',
    chocolate: 'd2691e',
    coral: 'ff7f50',
    cornflowerblue: '6495ed',
    cornsilk: 'fff8dc',
    crimson: 'dc143c',
    cyan: '00ffff',
    darkblue: '00008b',
    darkcyan: '008b8b',
    darkgoldenrod: 'b8860b',
    darkgray: 'a9a9a9',
    darkgreen: '006400',
    darkkhaki: 'bdb76b',
    darkmagenta: '8b008b',
    darkolivegreen: '556b2f',
    darkorange: 'ff8c00',
    darkorchid: '9932cc',
    darkred: '8b0000',
    darksalmon: 'e9967a',
    darkseagreen: '8fbc8f',
    darkslateblue: '483d8b',
    darkslategray: '2f4f4f',
    darkturquoise: '00ced1',
    darkviolet: '9400d3',
    deeppink: 'ff1493',
    deepskyblue: '00bfff',
    dimgray: '696969',
    dodgerblue: '1e90ff',
    feldspar: 'd19275',
    firebrick: 'b22222',
    floralwhite: 'fffaf0',
    forestgreen: '228b22',
    fuchsia: 'ff00ff',
    gainsboro: 'dcdcdc',
    ghostwhite: 'f8f8ff',
    gold: 'ffd700',
    goldenrod: 'daa520',
    gray: '808080',
    green: '008000',
    greenyellow: 'adff2f',
    honeydew: 'f0fff0',
    hotpink: 'ff69b4',
    indianred: 'cd5c5c',
    indigo: '4b0082',
    ivory: 'fffff0',
    khaki: 'f0e68c',
    lavender: 'e6e6fa',
    lavenderblush: 'fff0f5',
    lawngreen: '7cfc00',
    lemonchiffon: 'fffacd',
    lightblue: 'add8e6',
    lightcoral: 'f08080',
    lightcyan: 'e0ffff',
    lightgoldenrodyellow: 'fafad2',
    lightgrey: 'd3d3d3',
    lightgreen: '90ee90',
    lightpink: 'ffb6c1',
    lightsalmon: 'ffa07a',
    lightseagreen: '20b2aa',
    lightskyblue: '87cefa',
    lightslateblue: '8470ff',
    lightslategray: '778899',
    lightsteelblue: 'b0c4de',
    lightyellow: 'ffffe0',
    lime: '00ff00',
    limegreen: '32cd32',
    linen: 'faf0e6',
    magenta: 'ff00ff',
    maroon: '800000',
    mediumaquamarine: '66cdaa',
    mediumblue: '0000cd',
    mediumorchid: 'ba55d3',
    mediumpurple: '9370d8',
    mediumseagreen: '3cb371',
    mediumslateblue: '7b68ee',
    mediumspringgreen: '00fa9a',
    mediumturquoise: '48d1cc',
    mediumvioletred: 'c71585',
    midnightblue: '191970',
    mintcream: 'f5fffa',
    mistyrose: 'ffe4e1',
    moccasin: 'ffe4b5',
    navajowhite: 'ffdead',
    navy: '000080',
    oldlace: 'fdf5e6',
    olive: '808000',
    olivedrab: '6b8e23',
    orange: 'ffa500',
    orangered: 'ff4500',
    orchid: 'da70d6',
    palegoldenrod: 'eee8aa',
    palegreen: '98fb98',
    paleturquoise: 'afeeee',
    palevioletred: 'd87093',
    papayawhip: 'ffefd5',
    peachpuff: 'ffdab9',
    peru: 'cd853f',
    pink: 'ffc0cb',
    plum: 'dda0dd',
    powderblue: 'b0e0e6',
    purple: '800080',
    red: 'ff0000',
    rosybrown: 'bc8f8f',
    royalblue: '4169e1',
    saddlebrown: '8b4513',
    salmon: 'fa8072',
    sandybrown: 'f4a460',
    seagreen: '2e8b57',
    seashell: 'fff5ee',
    sienna: 'a0522d',
    silver: 'c0c0c0',
    skyblue: '87ceeb',
    slateblue: '6a5acd',
    slategray: '708090',
    snow: 'fffafa',
    springgreen: '00ff7f',
    steelblue: '4682b4',
    tan: 'd2b48c',
    teal: '008080',
    thistle: 'd8bfd8',
    tomato: 'ff6347',
    turquoise: '40e0d0',
    violet: 'ee82ee',
    violetred: 'd02090',
    wheat: 'f5deb3',
    white: 'ffffff',
    whitesmoke: 'f5f5f5',
    yellow: 'ffff00',
    yellowgreen: '9acd32'
};

interface IRgbParser
{
    regex: RegExp;
    apply: (match: any, rgbColor: Rgb) => void;
}

export interface IColor
{
    ColorType: any;
    isValid(): boolean;
    getValue(index: number): number;
    toString(): string;
}

var colorParsers: IRgbParser[] = [
    {
        // eg: 'rgb(1,2,3)'
        regex: /^rgb\((\d{1,3}),(\d{1,3}),(\d{1,3})\)$/,
        apply: function(match, rgbColor)
        {
            rgbColor.R = parseInt(match[1]) / 0xFF;
            rgbColor.G = parseInt(match[2]) / 0xFF;
            rgbColor.B = parseInt(match[3]) / 0xFF;
        }
    },
    {
        // eg: '001122'
        regex: /^[0-9a-f]{6}$/,
        apply: function(match, rgbColor)
        {
            var hex = match[0];
            rgbColor.R = parseInt(hex.substr(0, 2), 16) / 0xFF;
            rgbColor.G = parseInt(hex.substr(2, 2), 16) / 0xFF;
            rgbColor.B = parseInt(hex.substr(4, 2), 16) / 0xFF;
        }
    },
    {
        // eg: '123'
        regex: /^[0-9a-f]{3}$/,
        apply: function(match, rgbColor)
        {
            var hex = match[0];
            rgbColor.R = parseInt(hex[0] + hex[0], 16) / 0xFF;
            rgbColor.G = parseInt(hex[1] + hex[1], 16) / 0xFF;
            rgbColor.B = parseInt(hex[2] + hex[2], 16) / 0xFF;
        }
    }
];

export function blend(from: IColor, to: IColor, ratio: number)
{
    if (ratio <= 0)
        return from;
    if (ratio >= 1)
        return to;

    var from1 = from.getValue(0);
    var from2 = from.getValue(1);
    var from3 = from.getValue(2);

    var r = from1 + ratio * (to.getValue(0) - from1);
    var g = from2 + ratio * (to.getValue(1) - from2);
    var b = from3 + ratio * (to.getValue(2) - from3);

    return new from.ColorType(r, g, b);
}

export class Rgb implements IColor
{
    public R: number;
    public G: number;
    public B: number;

    public ColorType: any = Rgb;

    /**
     * Constructs a new colour in RGB space.
     *
     * @param r red level, between 0 and 1.
     * @param g green level, between 0 and 1.
     * @param b blue level, between 0 and 1.
     */
    constructor(r: any, g?: number, b?: number)
    {
        if (typeof(r) === 'string')
        {
            // seems that we were passed a string value instead of three numbers
            var colorString = r.toLowerCase();

            // remove all whitespace
            colorString = colorString.replace(/ /g, '');
            // remove any leading hash character
            colorString = colorString.replace(/^#/, '');

            // check for known colour names
            var knownColor = knownColors[colorString];
            if (knownColor) {
                // if found, convert to hex
                colorString = knownColor;
            }

            var self = this,
                parsedAnything = false;

            // loop through each of the colour parser definitions
            _.each(
                colorParsers,
                parser =>
                {
                    var match = parser.regex.exec(colorString);
                    if (match) {
                        parser.apply(match, self);
                        parsedAnything = true;
                    }
                }
            );

            if (!parsedAnything)
                throw new Error("Unable to parse '" + r + "' as a RGB colour.");
        }
        else
        {
            this.R = r;
            this.G = g;
            this.B = b;
        }

        if (!this.isValid()) {
            throw new Error('Values are out of range ' + this.R + ' ' + this.G + ' ' + this.B);
        }
    }

    public isValid()
    {
        return 0 <= this.R && this.R <= 1
            && 0 <= this.G && this.G <= 1
            && 0 <= this.B && this.B <= 1;
    }

    public getValue(index: number)
    {
        switch (index) {
            case 0:
                return this.R;
            case 1:
                return this.G;
            case 2:
                return this.B;
        }

        throw new Error("Index can only be between 0 and 2, inclusive, not: " + index);
    }

    public static blend(from: Rgb, to: Rgb, ratio: number)
    {
        if (ratio <= 0)
            return from;
        if (ratio >= 1)
            return to;

        var h = from.R + ratio * (to.R - from.R);
        var s = from.G + ratio * (to.G - from.G);
        var l = from.B + ratio * (to.B - from.B);

        return new Rgb(h, s, l);
    }

    public static random()
    {
        return new Rgb(Math.random(), Math.random(), Math.random());
    }

    public static fromContext2d(context: CanvasRenderingContext2D, x: number, y: number)
    {
        var pixelData = context.getImageData(x, y, 1, 1).data;

        var r = pixelData[0] / 255;
        var g = pixelData[1] / 255;
        var b = pixelData[2] / 255;

        return new Rgb(r, g, b);
    }

    public toString(alpha: number = 1)
    {
        if (alpha === 1.0)
        {
            // Use regular "#112233" form
            var r = Math.round(this.R * 255).toString(16);
            var g = Math.round(this.G * 255).toString(16);
            var b = Math.round(this.B * 255).toString(16);
            if (r.length == 1) r = '0' + r;
            if (g.length == 1) g = '0' + g;
            if (b.length == 1) b = '0' + b;
            return '#' + r + g + b;
        }
        else
        {
            // Use "rgba(32,64,128, 0.256)" form
            return 'rgba(' + Math.round(this.R * 255) + ','
                           + Math.round(this.G * 255) + ','
                           + Math.round(this.B * 255) + ','
                           + alpha + ')';
        }
    }

    public toByteObject()
    {
        var r = Math.round(this.R * 255);
        var g = Math.round(this.G * 255);
        var b = Math.round(this.B * 255);

        return {r: r, g: g, b: b};
    }

    public toHsv()
    {
        return new Hsv(this);
    }
}

export class Hsv implements IColor
{
    /** The 'hue', between 0 and 1. */
    public H: number;
    /** The 'saturation', between 0 and 1. */
    public S: number;
    /** The 'value', between 0 and 1. */
    public V: number;

    public ColorType: any = Hsv;

    constructor(h: any, s?: number, v?: number)
    {
        if (typeof(h) === 'string') {
            this.setFromRgb(new Rgb(h));
        } else if (h instanceof Rgb) {
            this.setFromRgb(h);
        } else {
            this.H = h;
            this.S = s;
            this.V = v;
        }

        if (!this.isValid()) {
            throw new Error('Values are out of range: ' + h + ' ' + s + ' ' + v);
        }
    }

    public setFromRgb(rgb: Rgb)
    {
        var r = rgb.R,
            g = rgb.G,
            b = rgb.B;

        var max = Math.max(r, g, b),
            min = Math.min(r, g, b),
            d = max - min;

        if (max == min) {
            // achromatic
            this.H = 0;
        } else {
            var h;
            switch (max) {
                case r:
                    h = (g - b) / d + (g < b ? 6 : 0);
                    break;
                case g:
                    h = (b - r) / d + 2;
                    break;
                case b:
                    h = (r - g) / d + 4;
                    break;
            }
            h /= 6;
            this.H = h;
        }

        this.S = max == 0 ? 0 : d / max;
        this.V = max;
    }

    public isValid()
    {
        return 0 <= this.H && this.H <= 1
            && 0 <= this.S && this.S <= 1
            && 0 <= this.V && this.V <= 1;
    }

    public toRgb()
    {
        var i = Math.floor(this.H * 6);
        var f = this.H * 6 - i;
        var p = this.V * (1 - this.S);
        var q = this.V * (1 - f * this.S);
        var t = this.V * (1 - (1 - f) * this.S);

        var r, g, b;

        switch (i % 6) {
            case 0:
                r = this.V; g = t; b = p;
                break;
            case 1:
                r = q; g = this.V; b = p;
                break;
            case 2:
                r = p; g = this.V; b = t;
                break;
            case 3:
                r = p; g = q; b = this.V;
                break;
            case 4:
                r = t; g = p; b = this.V;
                break;
            case 5:
                r = this.V; g = p; b = q;
                break;
        }

        return new Rgb(r, g, b);
    }

    public getValue(index: number)
    {
        switch (index) {
            case 0:
                return this.H;
            case 1:
                return this.S;
            case 2:
                return this.V;
        }
        throw new Error("Index can only be between 0 and 2, inclusive.");
    }

    public toString()
    {
        return this.toRgb().toString();
    }

    public static blend(from: Hsv, to: Hsv, ratio: number)
    {
        if (ratio <= 0)
            return from;
        if (ratio >= 1)
            return to;

        var h = from.H + ratio * (to.H - from.H);
        var s = from.S + ratio * (to.S - from.S);
        var v = from.V + ratio * (to.V - from.V);

        return new Hsv(h, s, v);
    }

    public static random()
    {
        return new Hsv(Math.random(), Math.random(), Math.random());
    }

    public static fromAngle(radians: number, s: number = 1, v: number = 1)
    {
        var twoPi = Math.PI*2;

        if (typeof(s) === 'undefined')
            s = 1;
        if (typeof(v) === 'undefined')
            v = 1;

        while (radians < 0) {
            radians += twoPi;
        } while (radians > twoPi) {
            radians -= twoPi;
        }

        return new Hsv(radians / twoPi, s, v);
    }
}

export class Hsl implements IColor
{
    public H: number;
    public S: number;
    public L: number;

    public ColorType: any = Hsl;

    constructor(h: any, s?: number, l?: number)
    {
        if (typeof(h) === 'string') {
            this.setFromRgb(new Rgb(h));
        } else if (h instanceof Rgb) {
            this.setFromRgb(h);
        } else {
            this.H = h;
            this.S = s;
            this.L = l;
        }

        if (!this.isValid()) {
            throw new Error('Values are out of range: ' + h + ' ' + s + ' ' + l);
        }
    }

    public setFromRgb(rgb: Rgb)
    {
        var r = rgb.R,
            g = rgb.G,
            b = rgb.B;

        var max = Math.max(r, g, b),
            min = Math.min(r, g, b);

        this.L = (max + min) / 2;

        if (max == min) {
            // achromatic
            this.H = this.S = 0;
        } else {
            var d = max - min;
            this.S = this.L > 0.5 ? d / (2 - max - min) : d / (max + min);
            switch (max) {
                case r:
                    this.H = (g - b) / d + (g < b ? 6 : 0);
                    break;
                case g:
                    this.H = (b - r) / d + 2;
                    break;
                case b:
                    this.H = (r - g) / d + 4;
                    break;
            }
            this.H /= 6;
        }
    }

    public isValid()
    {
        return 0 <= this.H && this.H <= 1
            && 0 <= this.S && this.S <= 1
            && 0 <= this.L && this.L <= 1;
    }

    private static hue2rgb(p: number, q: number, t: number)
    {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1 / 6) return p + (q - p) * 6 * t;
        if (t < 1 / 2) return q;
        if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
        return p;
    }

    public toRgb()
    {
        var r, g, b;
        var h = this.H, s = this.S, l = this.L;

        if (s == 0) {
            // achromatic
            r = g = b = l;
        } else {
            var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            var p = 2 * l - q;
            r = Hsl.hue2rgb(p, q, h + 1 / 3);
            g = Hsl.hue2rgb(p, q, h);
            b = Hsl.hue2rgb(p, q, h - 1 / 3);
        }

        return new Rgb(r, g, b);
    }

    public getValue(index: number)
    {
        switch (index) {
            case 0:
                return this.H;
            case 1:
                return this.S;
            case 2:
                return this.L;
        }
        throw new Error("Index can only be between 0 and 2, inclusive.");
    }

    public toString()
    {
        return this.toRgb().toString();
    }

    public static blend(from: Hsl, to: Hsl, ratio: number)
    {
        if (ratio <= 0)
            return from;
        if (ratio >= 1)
            return to;

        var h = from.H + ratio * (to.H - from.H);
        var s = from.S + ratio * (to.S - from.S);
        var l = from.L + ratio * (to.L - from.L);

        return new Hsl(h, s, l);
    }

    public static random()
    {
        return new Hsl(Math.random(), Math.random(), Math.random());
    }

    public static fromAngle(radians: number, s: number = 1, l: number = 1)
    {
        var twoPi = Math.PI*2;

        while (radians < 0) {
            radians += twoPi;
        } while (radians > twoPi) {
            radians -= twoPi;
        }

        return new Hsl(radians / twoPi, s, l);
    }
}
