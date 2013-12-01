/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'color/RgbColor'
    ],
    function(RgbColor)
    {
        /**
         * Converts RgbColor to HslColor.
         *
         * @param rgb RgbColor The color in RGB space
         * @return HslColor The color in HSL space
         */
        var setFromRgb = function(hsl, rgb)
        {
            var r = rgb.R,
                g = rgb.G,
                b = rgb.B;

            var max = Math.max(r, g, b),
                min = Math.min(r, g, b);

            hsl.L = (max + min) / 2;


            if (max == min) {
                // achromatic
                hsl.H = hsl.S = 0;
            } else {
                var d = max - min;
                hsl.S = l > 0.5 ? d / (2 - max - min) : d / (max + min);
                switch (max) {
                    case r:
                        hsl.H = (g - b) / d + (g < b ? 6 : 0);
                        break;
                    case g:
                        hsl.H = (b - r) / d + 2;
                        break;
                    case b:
                        hsl.H = (r - g) / d + 4;
                        break;
                }
                hsl.H /= 6;
            }
        };

        var HslColor = function(h, s, l)
        {
            if (typeof(h) === 'string') {
                var rgb = new RgbColor(h);
                setFromRgb(this, rgb);
            } else if (h instanceof RgbColor) {
                setFromRgb(this, h);
            } else {
                this.H = h;
                this.S = s;
                this.L = l;
            }

            if (!this.isValid()) {
                throw new Error('Values are out of range: ' + h + ' ' + s + ' ' + l);
            }
        };

        HslColor.prototype.isValid = function()
        {
            return 0 <= this.H && this.H <= 1
                && 0 <= this.S && this.S <= 1
                && 0 <= this.L && this.L <= 1;
        };

        var hue2rgb = function(p, q, t)
        {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1 / 6) return p + (q - p) * 6 * t;
            if (t < 1 / 2) return q;
            if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
            return p;
        };

        /**
         * Returns this color in RGB space.
         *
         * @return util.RgbColor The RGB representation
         */
        HslColor.prototype.toRgb = function()
        {
            var r, g, b;
            var h = this.H, s = this.S, l = this.L;

            if (s == 0) {
                // achromatic
                r = g = b = l;
            } else {
                var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
                var p = 2 * l - q;
                r = hue2rgb(p, q, h + 1 / 3);
                g = hue2rgb(p, q, h);
                b = hue2rgb(p, q, h - 1 / 3);
            }

            return new RgbColor(r, g, b);
        };

        //noinspection FunctionWithInconsistentReturnsJS
        HslColor.prototype.getValue = function(index)
        {
            if (index < 0 || index > 2)
                throw new Error("Index can only be between 0 and 2, inclusive.");
            switch (index) {
                case 0:
                    return this.H;
                case 1:
                    return this.S;
                case 2:
                    return this.L;
            }
        };

        HslColor.prototype.ColorType = HslColor;

        HslColor.prototype.toString = function()
        {
            return this.toRgb().toString();
        };

        ///////////////////////////////////////////////////////////////

        HslColor.blend = function(from, to, ratio)
        {
            if (ratio <= 0)
                return from;
            if (ratio >= 1)
                return to;

            var h = from.H + ratio * (to.H - from.H);
            var s = from.S + ratio * (to.S - from.S);
            var l = from.L + ratio * (to.L - from.L);

            return new HslColor(h, s, l);
        };

        HslColor.random = function()
        {
            return new HslColor(Math.random(), Math.random(), Math.random());
        };

        var twoPi = Math.PI*2;

        HslColor.fromAngle = function(radians, s, l)
        {
            if (typeof(s) === 'undefined')
                s = 1;
            if (typeof(l) === 'undefined')
                l = 1;

            while (radians < 0) {
                radians += twoPi;
            } while (radians > twoPi) {
                radians -= twoPi;
            }

            return new HslColor(radians / twoPi, s, l);
        };

        return HslColor;
    }
);