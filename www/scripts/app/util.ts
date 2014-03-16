/**
 * @author Drew Noakes
 */

export class Trackable<T>
{
    private callbacks: { (value: T, oldValue?: T): void; }[];

    constructor(private value: T = null)
    {
        this.callbacks = [];
    }

    public track(callback: (value: T, oldValue?: T) => void)
    {
        this.callbacks.push(callback);
        if (typeof (this.value) !== 'undefined' && this.value !== null)
            callback(this.value, undefined);
    }

    public setValue(value: T)
    {
        if (this.value === value)
            return;

        var oldValue = this.value;
        this.value = value;
        this.triggerChange(oldValue);
    }

    public getValue()
    {
        return this.value;
    }

    public triggerChange(oldValue?: T)
    {
        for (var i = 0; i < this.callbacks.length; i++) {
            this.callbacks[i](this.value, oldValue);
        }
    }

    public onchange(callback: ()=>void)
    {
        this.callbacks.push(callback);
    }
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

export function choose(...items: any[])
{
    var index = Math.floor(Math.random() * items.length);
    return items[index];
}

/** Calculate a linear interpolation between values. */
export function lerp(ratio: number, from: number, to: number)
{
    if (ratio <= 0)
        return from;
    if (ratio >= 1)
        return to;

    return from + ratio * (to - from);
}

export function arrayToHsla(hsl: number[], override?: { s?: number; l?: number })
{
    var s = override && override.s || hsl[1];
    var l = override && override.l || hsl[2];
    return 'hsla(' + hsl[0] + ',' + (s * 100) + '%,' + (l * 100) + '%,1)';
}

export function clearChildren(el: Element)
{
    while (el.hasChildNodes()) {
        el.removeChild(el.lastChild);
    }
}

export function clone<T>(obj: T): T
{
    return <T>JSON.parse(JSON.stringify(obj));
}
