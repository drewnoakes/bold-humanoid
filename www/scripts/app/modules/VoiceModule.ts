/**
 * @author Drew Noakes http://drewnoakes.com
 */

import control = require('control');
import Module = require('Module');

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

        var voiceControls = document.createElement('div');
        voiceControls.className = 'control-container flow';
        control.buildSettings('voice', voiceControls, this.closeables);
        element.appendChild(voiceControls);

        var vocaliserControls = document.createElement('div');
        vocaliserControls.className = 'control-container flow';
        control.buildSettings('vocaliser', vocaliserControls, this.closeables);
        element.appendChild(vocaliserControls);

        var sayings = document.createElement('div');
        sayings.className = 'control-container';
        control.buildActions('voice.speak', sayings);
        element.appendChild(sayings);
    }
}

export = VoiceModule;
