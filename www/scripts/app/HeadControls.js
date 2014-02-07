/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlBuilder'
    ],
    function (ControlBuilder)
    {
        'use strict';

        var HeadControls = function ()
        {
            this.element = document.createElement('div');
            this.element.className = "head-controls";

            ControlBuilder.action('head-module.move-left', this.element);
            ControlBuilder.action('head-module.move-up', this.element);
            ControlBuilder.action('head-module.move-down', this.element);
            ControlBuilder.action('head-module.move-right', this.element);

            ControlBuilder.action('head-module.move-home', this.element);
            ControlBuilder.action('head-module.move-zero', this.element);
        };

        return HeadControls;
    }
);