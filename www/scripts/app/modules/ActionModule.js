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

        var ActionModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'actions';
            this.id = 'actions';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];
        };

        ActionModule.prototype.load = function()
        {
            ControlBuilder.build('actions', $('<div></div>', {'class': 'control-container ambulator-controls'}).appendTo(this.$container));
        };

        ActionModule.prototype.unload = function()
        {
            this.$container.empty();
        };

        return ActionModule;
    }
);
