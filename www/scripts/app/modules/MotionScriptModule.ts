/**
 * @author Drew Noakes http://drewnoakes.com
 */

import control = require('control');
import Module = require('Module');

class MotionScriptModule extends Module
{
    constructor()
    {
        super('motion-scripts', 'motion scripts');
    }

    public load(element: HTMLDivElement)
    {
        var container = document.createElement('div');
        container.className = 'control-container ambulator-controls';
        element.appendChild(container);

        control.buildActions('motion-script', container);
    }
}

export = MotionScriptModule;
