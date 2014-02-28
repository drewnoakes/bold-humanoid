/**
 * @author Drew Noakes http://drewnoakes.com
 */

import control = require('control');
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
        control.buildSetting('options.announce-fsm-states', usage, this.closeables);
        element.appendChild(usage);

        var controls = document.createElement('div');
        controls.className = 'control-container flow';
        control.buildSettings('voice', controls, this.closeables);
        element.appendChild(controls);

        var sayings = document.createElement('div');
        sayings.className = 'control-container';
        control.buildActions('voice.speak', sayings);
        element.appendChild(sayings);
    }
}

export = VoiceModule;
