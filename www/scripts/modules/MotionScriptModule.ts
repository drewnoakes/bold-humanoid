/**
 * @author Drew Noakes http://drewnoakes.com
 */

import control = require('control');
import Module = require('Module');
import Trackable = require('util/Trackable');
import Checkbox = require('controls/Checkbox');

var common = [
    "sit-down", "sit-down-arms-back", "stand-ready", "stand-straight", "zero",
    "kick-cross-left", "kick-cross-right", "kick-left", "kick-right"
];

class MotionScriptModule extends Module
{
    private showAll: Trackable<boolean>;

    constructor()
    {
        super('motion-scripts', 'motion scripts');
    }

    public load(width: number)
    {
        this.showAll = new Trackable<boolean>(false);
        this.showAll.track(value => this.element.classList[value?'add':'remove']('show-all'));

        this.element.appendChild(new Checkbox('Show all', this.showAll).element);

        var container = document.createElement('div');
        container.className = 'control-container motion-script-controls';
        this.element.appendChild(container);

        control.buildActions('motion-script', container);

        var buttons = container.querySelectorAll('button');
        for (var i = 0; i < buttons.length; i++)
        {
            var button = <HTMLButtonElement>buttons[i];

            var scriptName = button.textContent;
            button.classList.add(common.indexOf(scriptName) !== -1 ? 'common' : 'rare');
        }
    }

    public unload()
    {
        delete this.showAll;
    }
}

export = MotionScriptModule;
