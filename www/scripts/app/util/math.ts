/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts"/>

export function degToRad(deg: number) : number
{
  return Math.PI * deg/180;
}

export class MovingAverage
{
    private values: number[] = [];
    private count: number;
    private nextIndex: number;
    private sum: number;

    constructor(private windowSize: number)
    {
        this.nextIndex = 0;
        this.sum = 0;
        this.count = 0;
    }

    public next(value: number)
    {
        console.assert(!isNaN(value));

        if (this.count !== this.windowSize)
        {
            this.values.push(value);
            this.count++;
        }
        else
        {
            this.sum -= this.values[this.nextIndex];
            this.values[this.nextIndex] = value;
        }
        this.nextIndex++;
        if (this.nextIndex === this.windowSize)
            this.nextIndex = 0;
        this.sum += value;
        return this.sum/this.count;
    }
}
