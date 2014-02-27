/**
 * @author Drew Noakes http://drewnoakes.com
 */

import constants = require('constants');
import TimingPane = require('modules/TimingPane');
import Module = require('Module');

class MotionTimingModule extends Module
{
    private pane: TimingPane;

    constructor()
    {
        super('motion-timing', 'motion timing');
        this.pane = new TimingPane(constants.protocols.motionTiming, 125/*fps*/);
    }

    public load(element: HTMLDivElement)
    {
        this.pane.load(element);
    }

    public unload()
    {
        this.pane.unload();
    }

    public onResized(width, height)
    {
        this.pane.onResized(width, height);
    }
}

export = MotionTimingModule;
