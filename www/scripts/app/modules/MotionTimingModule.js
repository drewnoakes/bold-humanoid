/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'modules/TimingPane',
        'constants'
    ],
    function(TimingPane, constants)
    {
        'use strict';

        var MotionTimingModule = function()
        {
            this.pane = new TimingPane(constants.protocols.motionTiming, 125/*fps*/);

            /////

            this.title = 'motion timing';
            this.id = 'motion-timing';
            this.element = this.pane.container;
        };

        MotionTimingModule.prototype.load = function()
        {
            this.pane.load();
        };

        MotionTimingModule.prototype.unload = function()
        {
            this.pane.unload();
        };

        MotionTimingModule.prototype.onResized = function(width, height)
        {
            this.pane.onResized(width, height);
        };

        return MotionTimingModule;
    }
);
