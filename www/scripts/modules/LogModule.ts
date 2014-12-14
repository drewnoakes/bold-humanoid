/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import constants = require('constants');
import data = require('data');
import DOMTemplate = require('DOMTemplate');
import ICloseable = require('ICloseable');
import Module = require('Module');
import state = require('state');

var moduleTemplate = DOMTemplate.forId('log-module-template');

var MAX_ROW_COUNT = 2000;

class LogModule extends Module
{
    private subscription: ICloseable;
    private list: HTMLUListElement;
    private scroller: HTMLDivElement;
    private autoScroll: HTMLInputElement;

    constructor()
    {
        super('log', 'log');
    }

    public load(width: number)
    {
        var templateRoot = <HTMLElement>moduleTemplate.create();

        this.element.appendChild(templateRoot);

        this.list = <HTMLUListElement>templateRoot.querySelector('ul.log-messages');
        this.scroller = <HTMLDivElement>templateRoot.querySelector('div.scroll-container');
        this.autoScroll = <HTMLInputElement>document.getElementById('log-autoscroll');

        this.subscription = new data.Subscription<any>(
            constants.protocols.log,
            {
                onmessage: this.onLogMessage.bind(this)
            }
        );

        templateRoot.querySelector('button.clear-log').addEventListener('click', () =>
        {
            while (this.list.childElementCount > 0)
                this.list.removeChild(this.list.lastChild);
        });
    }

    public unload()
    {
        delete this.list;
        delete this.scroller;

        if (this.subscription)
            this.subscription.close();
    }

    private onLogMessage(data: any)
    {
        var appendMessage = (log: state.LogMessage) =>
        {
            while (this.list.childElementCount > MAX_ROW_COUNT)
                this.list.removeChild(this.list.firstChild);

            var li = document.createElement('li');
            li.className = 'level-' + log.lvl;

            var scope = document.createElement('span');
            scope.textContent = log.scope;
            li.appendChild(scope);

            li.appendChild(document.createTextNode(log.msg));

            this.list.appendChild(li);

            // Scroll to bottom
            if (this.autoScroll.checked)
              this.scroller.scrollTop = this.list.offsetHeight - this.scroller.offsetHeight + 5;
        };

        if (data instanceof Array)
        {
            for (var i = 0; i < data.length; i++)
                appendMessage(<state.LogMessage>data[i]);
        }
        else
        {
            appendMessage(<state.LogMessage>data);
        }
    }
}

export = LogModule;
