/**
 * @author Drew Noakes http://drewnoakes.com
 */

import Closeable = require('util/Closeable');
import util = require('util');

class Module
{
    public closeables: Closeable = new Closeable();

    constructor(public id:string, public title:string)
    {}

    public load(element: HTMLDivElement)
    {}

    public unload()
    {}

    public onResized(width: number, height: number)
    {}
}

export = Module;
