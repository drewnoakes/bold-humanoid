/**
 * @author Drew Noakes http://drewnoakes.com
 */

import ControlBuilder = require('ControlBuilder');
import Module = require('../Module');

class VoiceModule extends Module
{
    constructor()
    {
        super('voice', 'voice');
    }

    public load(element: HTMLDivElement)
    {
        var usage = document.createElement('div');
        usage.className = 'control-container';
        this.closeables.add(ControlBuilder.build('options.announce-fsm-states', usage));
        element.appendChild(usage);

        var controls = document.createElement('div');
        controls.className = 'control-container flow';
        ControlBuilder.buildAll('voice', controls);
        element.appendChild(controls);

        var sayings = document.createElement('div');
        sayings.className = 'control-container';
        ControlBuilder.actions('voice.speak', sayings);
        element.appendChild(sayings);
    }
}

export = VoiceModule;