/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import ICloseable = require('ICloseable');

class Closeable
{
    private closeables: ICloseable[] = [];

    public add(closeable: ICloseable)
    {
        this.closeables.push(closeable);
    }

    public closeAll()
    {
        _.each(this.closeables, closeable => closeable.close());
        this.closeables = [];
    }
}

export = Closeable;
