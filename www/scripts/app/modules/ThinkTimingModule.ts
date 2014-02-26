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

        var ThinkTimingModule = function()
        {
            this.pane = new TimingPane(constants.protocols.thinkTiming, 30/*fps*/);

            /////

            this.title = 'think timing';
            this.id = 'think-timing';
            this.element = this.pane.container;
        };

        ThinkTimingModule.prototype.load = function()
        {
            this.pane.load();
        };

        ThinkTimingModule.prototype.unload = function()
        {
            this.pane.unload();
        };

        ThinkTimingModule.prototype.onResized = function(width, height)
        {
            this.pane.onResized(width, height);
        };

        return ThinkTimingModule;
    }
);
