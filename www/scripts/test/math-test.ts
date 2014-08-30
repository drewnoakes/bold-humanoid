/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../libs/jasmine.d.ts" />

import math = require('scripts/app/util/math');

describe("clamp", () =>
{
    it("ensures value doesn't fall outside range", () =>
    {
        expect(math.clamp(1, 0, 2)).toEqual(1);
        expect(math.clamp(0, 0, 2)).toEqual(0);
        expect(math.clamp(2, 0, 2)).toEqual(2);

        expect(math.clamp(-1, 0, 2)).toEqual(0);
        expect(math.clamp(3, 0, 2)).toEqual(2);
    });
});

describe("lerp", () =>
{
    it("interpolates correctly", () =>
    {
        expect(math.lerp(0.0, 5, 10)).toEqual(5);
        expect(math.lerp(1.0, 5, 10)).toEqual(10);
        expect(math.lerp(0.5, 5, 10)).toEqual(7.5);
    });

    it("extrapolates correctly", () =>
    {
        expect(math.lerp(-1.0, 5, 10)).toEqual(0);
        expect(math.lerp(2.0, 5, 10)).toEqual(15);
    });
});
