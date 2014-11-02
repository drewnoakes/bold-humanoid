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
        super('think-timing', 'think timing', {fullScreen: true});
        this.pane = new TimingPane(constants.protocols.thinkTimingState, 30/*fps*/);
    }

    public load(width: number)
    {
        this.pane.load(this.element);
        this.pane.onResized(width, this.isFullScreen.getValue());
    }

    public unload()
    {
        this.pane.unload();
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        this.pane.onResized(width, isFullScreen);
    }
}

export = ThinkTimingModule;
