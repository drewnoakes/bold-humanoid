/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import control = require('control');
import DOMTemplate = require('DOMTemplate');
import Module = require('Module');
import TabControl = require('util/TabControl');

var moduleTemplate = new DOMTemplate('config-module-template');

class ConfigModule extends Module
{
    private settingTextElement: HTMLDivElement;
    private actionTextElement: HTMLDivElement;
    private filter: HTMLInputElement;

    constructor()
    {
        super('config', 'config');
    }

    public load(element: HTMLDivElement)
    {
        var templateRoot = <HTMLDListElement>moduleTemplate.create();

        element.appendChild(templateRoot);

        this.settingTextElement = <HTMLDivElement>templateRoot.querySelector('dd.settings > div.json-text');
        this.actionTextElement = <HTMLDivElement>templateRoot.querySelector('dd.actions > div.json-text');

        this.closeables.add(control.onSettingChange(this.updateSettingText.bind(this)));

        var header = templateRoot.querySelector('dd.settings > div.header');
        control.buildActions('config', header);

        this.filter = document.createElement('input');
        this.filter.type = 'text';
        this.filter.placeholder = 'Type to filter...';
        this.filter.addEventListener('input', this.updateSettingText.bind(this));
        header.appendChild(this.filter);

        this.updateSettingText();

        this.actionTextElement.textContent = control.getActionText();

        new TabControl(templateRoot);
    }

    public unload()
    {
        delete this.settingTextElement;
    }

    private updateSettingText()
    {
        var matching = this.filter.value;
        this.settingTextElement.textContent = control.getSettingText(matching);
    }
}

export = ConfigModule;
