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

        var ThinkTimingModule = function()
        {
            this.$container = $('<div></div>');

            this.pane = new TimingPane(Protocols.thinkTiming, 30);

            /////

            this.title = 'think timing';
            this.id = 'think-timing';
            this.panes = [
                {
                    title: 'main',
                    element: this.pane.container,
                    onResized: _.bind(this.pane.onResized, this.pane),
                    supports: { fullScreen: false }
                }
            ];
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
