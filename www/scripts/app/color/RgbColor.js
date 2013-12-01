/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function()
    {
        var RgbColor = function(r, g, b)
        {
            if (typeof(r) === 'string') {
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
                    function(parser)
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
            } else {
                this.R = r;
                this.G = g;
                this.B = b;
            }

            if (!this.isValid()) {
                throw new Error('Values are out of range ' + r + ' ' + g + ' ' + b);
            }
        };

        RgbColor.prototype.isValid = function()
        {
            return 0 <= this.R && this.R <= 1
                && 0 <= this.G && this.G <= 1
                && 0 <= this.B && this.B <= 1;
        };

        //noinspection FunctionWithInconsistentReturnsJS
        RgbColor.prototype.getValue = function(index)
        {
            if (index < 0 || index > 2)
                throw new Error("Index can only be between 0 and 2, inclusive.");
            switch (index) {
                case 0:
                    return this.R;
                case 1:
                    return this.G;
                case 2:
                    return this.B;
            }
        };

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

        var colorParsers = [
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

        RgbColor.blend = function(from, to, ratio)
        {
            if (ratio <= 0)
                return from;
            if (ratio >= 1)
                return to;

            var h = from.R + ratio * (to.R - from.R);
            var s = from.G + ratio * (to.G - from.G);
            var l = from.B + ratio * (to.B - from.B);

            return new RgbColor(h, s, l);
        };

        RgbColor.random = function()
        {
            return new RgbColor(Math.random(), Math.random(), Math.random());
        };

        RgbColor.prototype.ColorType = RgbColor;

        RgbColor.prototype.toString = function()
        {
            var r = Math.round(this.R * 255).toString(16);
            var g = Math.round(this.G * 255).toString(16);
            var b = Math.round(this.B * 255).toString(16);
            if (r.length == 1) r = '0' + r;
            if (g.length == 1) g = '0' + g;
            if (b.length == 1) b = '0' + b;
            return '#' + r + g + b;
        };

        RgbColor.prototype.toByteObject = function()
        {
            var r = Math.round(this.R * 255);
            var g = Math.round(this.G * 255);
            var b = Math.round(this.B * 255);
            return {r: r, g: g, b: b};
        };

        return RgbColor;
    }
);