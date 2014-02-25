/**
 * @author Drew Noakes http://drewnoakes.com
 */

import ControlBuilder = require('ControlBuilder');

class HeadControls
{
    public element: HTMLDivElement;

    constructor()
    {
        this.element = document.createElement('div');
        this.element.className = "head-controls";

        ControlBuilder.action('head-module.move-left', this.element);
        ControlBuilder.action('head-module.move-up', this.element);
        ControlBuilder.action('head-module.move-down', this.element);
        ControlBuilder.action('head-module.move-right', this.element);

        ControlBuilder.action('head-module.move-home', this.element);
        ControlBuilder.action('head-module.move-zero', this.element);
    }
}

export = HeadControls;
