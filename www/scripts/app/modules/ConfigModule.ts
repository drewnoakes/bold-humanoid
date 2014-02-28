/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import control = require('control');
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

        this.closeables.add(control.onSettingChange(this.updateText.bind(this)));

        var header = templateRoot.querySelector('div.header');
        control.buildActions('config', header);

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
        this.textElement.textContent = control.getConfigText(matching);
    }
}

export = ConfigModule;
