/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import ICloseable = require('ICloseable');
import Module = require('Module');

var moduleTemplate = DOMTemplate.forId('state-dump-module-template');

class StateDumpModule extends Module
{
    private subscription: ICloseable;
    private textElement: HTMLDivElement;

    constructor()
    {
        super('state', 'state dump');
    }

    public load(width: number)
    {
        var templateRoot = <HTMLElement>moduleTemplate.create();

        this.element.appendChild(templateRoot);

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
                        onmessage: this.onState.bind(this)
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

    private onState(data: any)
    {
        this.textElement.textContent = JSON.stringify(data, undefined, 2);
    }
}

export = StateDumpModule;
