/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import data = require('data');
import control = require('control');
import Closeable = require('util/Closeable');
import state = require('state');
import Module = require('Module');

class OptionTreeModule extends Module
{
    private optionElementByName: {[name:string]:HTMLLIElement} = {};
    private optionList: HTMLUListElement;

    constructor()
    {
        super('optiontree', 'option-tree');
    }

    public load(element: HTMLDivElement)
    {
        this.optionList = document.createElement('ul');
        this.optionList.className = 'options';
        element.appendChild(this.optionList);

        var usage = document.createElement('div');
        usage.className = 'control-container';
        control.buildSetting('options.announce-fsm-states', usage, this.closeables);
        element.appendChild(usage);

        this.closeables.add(new data.Subscription<state.OptionTree>(
            constants.protocols.optionTreeState,
            {
                onmessage: this.onData.bind(this)
            }
        ));
    }

    public unload()
    {
        delete this.optionList;
    }

    private onData(data: state.OptionTree)
    {
        _.each(this.optionElementByName, optionDiv => optionDiv.className = '');

        for (var i = 0; i < data.ranoptions.length; i++)
        {
            var name = data.ranoptions[i],
                optionDiv = this.optionElementByName[name];

            if (optionDiv) {
                optionDiv.classList.add('ran');
            } else {
                optionDiv = document.createElement('li');
                optionDiv.textContent = name;
                optionDiv.className = 'ran';
                this.optionElementByName[name] = optionDiv;
                this.optionList.appendChild(optionDiv);
            }
        }
    }
}

export = OptionTreeModule;
