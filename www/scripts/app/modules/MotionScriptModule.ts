/**
 * @author Drew Noakes http://drewnoakes.com
 */

import ControlBuilder = require('ControlBuilder');
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

        ControlBuilder.actions('motion-script', container);
    }
}

export = MotionScriptModule;
