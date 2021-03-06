/**
 * @author Drew Noakes https://drewnoakes.com
 */

import IObservable = require('IObservable');
import ICloseable = require('ICloseable');

class Trackable<T>
{
    private callbacks: { (value: T, oldValue?: T): void; }[];

    constructor(private value: T = null)
    {
        this.callbacks = [];
    }

    public track(callback: (value: T, oldValue?: T) => void): ICloseable
    {
        this.callbacks.push(callback);
        if (typeof (this.value) !== 'undefined' && this.value !== null)
            callback(this.value, undefined);

        return { close: () => this.removeCallback(callback) };
    }

    public removeCallback(callback: (value: T, oldValue?: T) => void)
    {
        var index = this.callbacks.indexOf(callback);
        if (index === -1)
        {
            console.warn("Attempt to remove an unregistered callback from Trackable");
            return false;
        }
        this.callbacks.splice(index, 1);
        return true;
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

    public onchange(callback: (value: T)=>void): ICloseable
    {
        this.callbacks.push(callback);

        return { close: () => this.removeCallback(callback) };
    }
}

export = Trackable;
