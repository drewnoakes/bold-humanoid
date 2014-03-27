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

    public load()
    {
        var container = document.createElement('div');
        container.className = 'control-container ambulator-controls';
        this.element.appendChild(container);

        control.buildActions('motion-script', container);
    }
}

export = MotionScriptModule;
