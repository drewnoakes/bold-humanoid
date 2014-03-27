/**
 * @author Drew Noakes https://drewnoakes.com
 */

class Trackable<T>
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

    public onchange(callback: (value: T)=>void)
    {
        this.callbacks.push(callback);
    }
}

export = Trackable;
