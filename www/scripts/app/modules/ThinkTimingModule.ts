/**
 * @author Drew Noakes http://drewnoakes.com
 */

import constants = require('constants');
import TimingPane = require('modules/TimingPane');
import Module = require('Module');

class ThinkTimingModule extends Module
{
    private pane: TimingPane;

    constructor()
    {
        super('think-timing', 'think timing');
        this.pane = new TimingPane(constants.protocols.thinkTiming, 30/*fps*/);
    }

    public load()
    {
        this.pane.load(this.element);
    }

    public unload()
    {
        this.pane.unload();
    }

    public onResized(width: number, height: number)
    {
        this.pane.onResized(width, height);
    }
}

export = ThinkTimingModule;
