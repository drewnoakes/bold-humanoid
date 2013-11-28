/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlBuilder'
    ],
    function(ControlBuilder)
    {
        'use strict';

        var MotionScriptModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'motion scripts';
            this.id = 'motion-scripts';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];
        };

        MotionScriptModule.prototype.load = function()
        {
            ControlBuilder.actions('motion-script', $('<div></div>', {'class': 'control-container ambulator-controls'}).appendTo(this.$container).get(0));
        };

        MotionScriptModule.prototype.unload = function()
        {
            this.$container.empty();
        };

        return MotionScriptModule;
    }
);
