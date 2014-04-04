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

    public load()
    {
        var usage = document.createElement('div');
        usage.className = 'control-container';
        control.buildSetting('options.announce-fsm-states', usage, this.closeables);
        control.buildSetting('role-decider.announce-roles', usage, this.closeables);
        this.element.appendChild(usage);

        var voiceControls = document.createElement('div');
        voiceControls.className = 'control-container flow';
        control.buildSettings('voice', voiceControls, this.closeables);
        this.element.appendChild(voiceControls);

        var vocaliserControls = document.createElement('div');
        vocaliserControls.className = 'control-container flow';
        control.buildSettings('vocaliser', vocaliserControls, this.closeables);
        this.element.appendChild(vocaliserControls);

        var sayings = document.createElement('div');
        sayings.className = 'control-container';
        control.buildActions('voice.speak', sayings);
        this.element.appendChild(sayings);
    }
}

export = VoiceModule;
