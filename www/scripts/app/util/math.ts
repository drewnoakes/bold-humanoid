/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts"/>

export function radToDeg(rad: number)
{
    return 180.0 * rad / Math.PI;
}

export function degToRad(deg: number)
{
    return Math.PI * deg / 180.0;
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

export function clamp(val: number, min: number, max: number)
{
    return val < min
        ? min
        : val > max
            ? max
            : val;
}

export function lerp(ratio: number, lowerOutput: number, upperOutput: number)
{
    return lowerOutput + (upperOutput - lowerOutput) * ratio;
}

export function roundUpHumane(value: number)
{
    // A function for nicely rounding numbers up for human beings.
    // Eg: 180.2 -> 200
    //       3.5 -> 5
    //       8.9 -> 10

    // calculate the magnitude of the value
    var mag = Math.floor(Math.log(value) / Math.log(10));
    var magPow = Math.pow(10, mag);

    // calculate the most significant digit of the value
    var magMsd = Math.ceil(value / magPow);

    // promote the MSD to either 1, 2, or 5
    if (magMsd > 5.0)
        magMsd = 10.0;
    else if (magMsd > 2.0)
        magMsd = 5.0;
    else if (magMsd > 1.0)
        magMsd = 2.0;

    return magMsd * magPow;
}

export function roundDownHumane(value: number)
{
    // A function for nicely rounding numbers down for human beings.
    // Eg: 180.2 -> 100
    //       3.5 -> 2
    //       8.9 -> 5

    // calculate the magnitude of the value
    var mag = Math.floor(Math.log(value) / Math.log(10));
    var magPow = Math.pow(10, mag);

    // calculate the most significant digit of the value
    var magMsd = Math.ceil(value / magPow);

    // demote the MSD to either 1, 2, or 5
    if (magMsd > 5.0)
        magMsd = 5.0;
    else if (magMsd > 2.0)
        magMsd = 2.0;
    else if (magMsd > 1.0)
        magMsd = 1.0;

    return magMsd * magPow;
}

// NOTE quaternion values are arranges as [x,y,z,w]

export function yawFromQuaternion(quaternion: number[]) : number
{
  return Math.asin(2*quaternion[0]*quaternion[1] + 2*quaternion[2]*quaternion[3]);
}

export function rollFromQuaternion(quaternion: number[]) : number
{
  return Math.atan2(
    2*quaternion[1]*quaternion[3] - 2*quaternion[0]*quaternion[2],
    1 - 2*quaternion[1]*quaternion[1] - 2*quaternion[2]*quaternion[2]);
}

export function pitchFromQuaternion(quaternion: number[]) : number
{
  return Math.atan2(
    2*quaternion[0]*quaternion[3] - 2*quaternion[1]*quaternion[2],
    1 - 2*quaternion[0]*quaternion[0] - 2*quaternion[2]*quaternion[2]);
}
