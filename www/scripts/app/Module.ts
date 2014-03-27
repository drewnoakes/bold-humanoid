/**
 * @author Drew Noakes http://drewnoakes.com
 */

import Closeable = require('util/Closeable');
import Trackable = require('util/Trackable');

class Module
{
    public closeables: Closeable = new Closeable();
    public isFullScreen: Trackable<boolean> = new Trackable<boolean>(false);

    constructor(public id:string, public title:string, public options?: { fullScreen: boolean })
    {}

    public load(element: HTMLDivElement)
    {}

    public unload()
    {}

    public onResized(width: number, height: number)
    {}
}

export = Module;
