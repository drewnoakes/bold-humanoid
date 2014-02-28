/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import Module = require('Module');

var moduleTemplate = new DOMTemplate('state-dump-module-template');

interface ICloseable
{
    close();
}

class StateDumpModule extends Module
{
    private subscription: ICloseable;
    private textElement: HTMLDivElement;

    constructor()
    {
        super('state', 'state dump');
    }

    public load(element: HTMLDivElement)
    {
        var templateRoot = <HTMLElement>moduleTemplate.create();

        element.appendChild(templateRoot);

        this.textElement = <HTMLDivElement>templateRoot.querySelector('div.json-text');

        var select = <HTMLSelectElement>templateRoot.querySelector('select');

        var none = document.createElement('option');
        none.value = '';
        none.text = '(None)';
        select.appendChild(none);

        _.each(constants.allStateProtocols, stateName =>
        {
            var option = document.createElement('option');
            option.value = stateName;
            option.text = stateName;
            select.appendChild(option);
        });

        select.addEventListener('change', () =>
        {
            var protocol = select.options[select.selectedIndex].value;

            if (this.subscription)
                this.subscription.close();

            this.textElement.textContent = protocol ? 'Waiting for an update...' : '';

            if (protocol !== '') {
                this.subscription = new data.Subscription<any>(
                    protocol,
                    {
                        onmessage: this.onData.bind(this)
                    }
                );
            }
        });
    }

    public unload()
    {
        delete this.textElement;

        if (this.subscription)
            this.subscription.close();
    }

    private onData(data: any)
    {
        this.textElement.textContent = JSON.stringify(data, undefined, 2);
    }
}

export = StateDumpModule;
