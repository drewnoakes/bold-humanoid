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
