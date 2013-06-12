/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'modules/TimingPane',
        'Protocols'
    ],
    function(TimingPane, Protocols)
    {
        'use strict';

        var MotionTimingModule = function()
        {
            this.$container = $('<div></div>');

            this.pane = new TimingPane(Protocols.motionTiming, 8);

            /////

            this.title = 'motion timing';
            this.id = 'motion-timing';
            this.panes = [
                {
                    title: 'main',
                    element: this.pane.container,
                    onResized: _.bind(this.pane.onResized, this.pane),
                    supports: { fullScreen: false }
                }
            ];
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
