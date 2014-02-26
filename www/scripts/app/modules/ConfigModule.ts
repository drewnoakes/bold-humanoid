/**
 * @author Drew Noakes http://drewnoakes.com
 */

import ControlClient = require('ControlClient');
import ControlBuilder = require('ControlBuilder');
import DOMTemplate = require('DOMTemplate');
import Module = require('Module');

var moduleTemplate = new DOMTemplate('config-module-template');

class ConfigModule extends Module
{
    private textElement: HTMLDivElement;
    private filter: HTMLInputElement;

    constructor()
    {
        super('config', 'config');
    }

    public load(element: HTMLDivElement)
    {
        var templateRoot = <HTMLElement>moduleTemplate.create();

        element.appendChild(templateRoot);

        this.textElement = <HTMLDivElement>templateRoot.querySelector('div.json-text');

        this.closeables.add(ControlClient.onSettingChange(this.updateText.bind(this)));

        var header = templateRoot.querySelector('div.header');
        ControlBuilder.actions('config', header);

        this.filter = document.createElement('input');
        this.filter.type = 'text';
        this.filter.placeholder = 'Type to filter...';
        this.filter.addEventListener('input', this.updateText.bind(this));
        header.appendChild(this.filter);

        this.updateText();
    }

    public unload()
    {
        delete this.textElement;
    }

    private updateText()
    {
        var matching = this.filter.value;
        this.textElement.textContent = ControlClient.getConfigText(matching);
    }
}

export = ConfigModule;
