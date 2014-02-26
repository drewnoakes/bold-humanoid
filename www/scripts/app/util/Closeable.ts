/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

class Closeable
{
    private functions: {():void}[] = [];

    public add(obj)
    {
        if (obj instanceof Array)
        {
            _.each(obj, o => this.add(o));
        }
        else if (obj instanceof Function)
        {
            this.functions.push(obj);
        }
        else if (obj instanceof Object && obj.close instanceof Function)
        {
            this.functions.push(<any>obj.close);
        }
        else
        {
            console.error('Unexpected closeable registered', obj);
        }
    }

    public closeAll()
    {
        _.each(this.functions, fun => fun());

        this.functions = [];
    }
}

export = Closeable;
