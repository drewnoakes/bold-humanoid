/**
* @author Drew Noakes http://drewnoakes.com
*/
define(["require", "exports"], function(require, exports) {
    

    //export interface ISize
    //{
    //    width: number;
    //    height: number;
    //}
    //
    //export interface IBounds
    //{
    //    north: number;
    //    south: number;
    //    east: number;
    //    west: number;
    //}
    var Transform = (function () {
        function Transform() {
            this.reset();
        }
        /** Resets this matrix to the identity matrix. */
        Transform.prototype.reset = function () {
            this.m = [1, 0, 0, 1, 0, 0];
        };

        /** Sets <c>this = this * other</c> */
        Transform.prototype.multiply = function (other) {
            var m11 = this.m[0] * other.m[0] + this.m[2] * other.m[1];
            var m12 = this.m[1] * other.m[0] + this.m[3] * other.m[1];

            var m21 = this.m[0] * other.m[2] + this.m[2] * other.m[3];
            var m22 = this.m[1] * other.m[2] + this.m[3] * other.m[3];

            var dx = this.m[0] * other.m[4] + this.m[2] * other.m[5] + this.m[4];
            var dy = this.m[1] * other.m[4] + this.m[3] * other.m[5] + this.m[5];

            this.m[0] = m11;
            this.m[1] = m12;
            this.m[2] = m21;
            this.m[3] = m22;
            this.m[4] = dx;
            this.m[5] = dy;

            return this;
        };

        Transform.prototype.invert = function () {
            var d = 1 / (this.m[0] * this.m[3] - this.m[1] * this.m[2]);
            var m0 = this.m[3] * d;
            var m1 = -this.m[1] * d;
            var m2 = -this.m[2] * d;
            var m3 = this.m[0] * d;
            var m4 = d * (this.m[2] * this.m[5] - this.m[3] * this.m[4]);
            var m5 = d * (this.m[1] * this.m[4] - this.m[0] * this.m[5]);
            this.m[0] = m0;
            this.m[1] = m1;
            this.m[2] = m2;
            this.m[3] = m3;
            this.m[4] = m4;
            this.m[5] = m5;

            return this;
        };

        /**
        * Adjusts the rotation component of this matrix by the specified number of radians.
        */
        Transform.prototype.rotate = function (rad) {
            var c = Math.cos(rad);
            var s = Math.sin(rad);
            var m11 = this.m[0] * c + this.m[2] * s;
            var m12 = this.m[1] * c + this.m[3] * s;
            var m21 = this.m[0] * -s + this.m[2] * c;
            var m22 = this.m[1] * -s + this.m[3] * c;
            this.m[0] = m11;
            this.m[1] = m12;
            this.m[2] = m21;
            this.m[3] = m22;

            return this;
        };

        Transform.prototype.translate = function (x, y) {
            this.m[4] += this.m[0] * x + this.m[2] * y;
            this.m[5] += this.m[1] * x + this.m[3] * y;

            return this;
        };

        Transform.prototype.scale = function (x, y) {
            this.m[0] *= x;
            this.m[1] *= x;
            this.m[2] *= y;
            this.m[3] *= y;

            return this;
        };

        Transform.prototype.transformPoint = function (x, y) {
            var xn = x * this.m[0] + y * this.m[2] + this.m[4];
            var yn = x * this.m[1] + y * this.m[3] + this.m[5];
            return { x: xn, y: yn };
        };

        Transform.prototype.clone = function () {
            var transform = new Transform();
            transform.m[0] = this.m[0];
            transform.m[1] = this.m[1];
            transform.m[2] = this.m[2];
            transform.m[3] = this.m[3];
            transform.m[4] = this.m[4];
            transform.m[5] = this.m[5];
            return transform;
        };

        Transform.prototype.getScale = function () {
            return Math.sqrt(Math.pow(this.m[0], 2) + Math.pow(this.m[1], 2));
        };

        Transform.prototype.getRotation = function () {
            return Math.atan2(this.m[3], this.m[2]);
        };

        // TODO rename applyToCanvas2DContext
        Transform.prototype.applyTo = function (context) {
            context.setTransform(this.m[0], this.m[1], this.m[2], this.m[3], this.m[4], this.m[5]);
        };

        Transform.prototype.applyToSvg = function (g) {
            g.setAttribute('transform', 'matrix(' + this.m[0] + ' ' + this.m[1] + ' ' + this.m[2] + ' ' + this.m[3] + ' ' + this.m[4] + ' ' + this.m[5] + ')');
        };

        Transform.prototype.toString = function () {
            return '[' + this.m[0].toFixed(2) + ' ' + this.m[2].toFixed(2) + ' ' + this.m[4].toFixed(2) + ']\n' + '[' + this.m[1].toFixed(2) + ' ' + this.m[3].toFixed(2) + ' ' + this.m[5].toFixed(2) + ']\n' + '[0.00 0.00 1.00]\n';
        };
        return Transform;
    })();
    exports.Transform = Transform;
});
//# sourceMappingURL=geometry.js.map
