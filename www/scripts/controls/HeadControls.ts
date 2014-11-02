/**
 * @author Drew Noakes http://drewnoakes.com
 */

import control = require('control');

class HeadControls
{
    public element: HTMLDivElement;

    constructor()
    {
        this.element = document.createElement('div');
        this.element.className = "head-controls";

        control.buildAction('head-module.move-left', this.element);
        control.buildAction('head-module.move-up', this.element);
        control.buildAction('head-module.move-down', this.element);
        control.buildAction('head-module.move-right', this.element);

        control.buildAction('head-module.move-home', this.element);
        control.buildAction('head-module.move-zero', this.element);
    }
}

export = HeadControls;
