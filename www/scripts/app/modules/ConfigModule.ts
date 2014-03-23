/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import control = require('control');
import Module = require('Module');
import TabControl = require('controls/TabControl');

class ConfigModule extends Module
{
    private settingList: HTMLUListElement;
    private actionList: HTMLUListElement;
    private filter: HTMLInputElement;
    private tabControl: TabControl;

    constructor()
    {
        super('config', 'config');
    }

    public load(element: HTMLDivElement)
    {
        /*
            <div class="filter">
                <input type="text" placeholder="Type to filter..." />
            </div>
            <dl class="tab-control">
                <dt>settings</dt>
                <dd class="settings">
                    <div class="header"></div>
                    <ul></ul>
                </dd>
                <dt>actions</dt>
                <dd class="actions">
                    <ul></ul>
                </dd>
            </dl>
        */
        var filter = document.createElement('input');
        filter.className = 'filter';
        filter.type = 'text';
        filter.placeholder = 'Type to filter...';
        filter.addEventListener('input', () => this.setFilterText(filter.value));
        element.appendChild(filter);

        var dl = document.createElement('dl');
        dl.className = 'tab-control';

        /////////////////////// SETTINGS

        var settingsTab = document.createElement('dt');
        settingsTab.textContent = 'settings';
        dl.appendChild(settingsTab);

        var settingsPane = document.createElement('dd');
        settingsPane.className = 'settings';
        dl.appendChild(settingsPane);

        var header = document.createElement('div');
        header.className = 'header';
        settingsPane.appendChild(header);
        control.buildActions('config', header);

        this.settingList = document.createElement('ul');
        settingsPane.appendChild(this.settingList);

        _.each(control.getAllSettings(), setting =>
        {
            if (setting.isReadOnly)
                return;

            var li = document.createElement('li');
            li.dataset['path'] = setting.path;
            // TODO add more details here
            control.createSettingControl(setting, li, this.closeables, /*hideLabel*/true);
            var desc = document.createElement('span');
            desc.className = 'description';
            desc.textContent = setting.path;
            li.appendChild(desc);
            this.settingList.appendChild(li);
        });

        /////////////////////// ACTIONS

        var actionsTab = document.createElement('dt');
        actionsTab.textContent = 'actions';
        dl.appendChild(actionsTab);

        var actionsPane = document.createElement('dd');
        actionsPane.className = 'actions';
        dl.appendChild(actionsPane);

        this.actionList = document.createElement('ul');
        actionsPane.appendChild(this.actionList);

        _.each(control.getAllActions(), action =>
        {
            if (action.hasArguments)
                return;

            var li = document.createElement('li');
            li.dataset['path'] = action.id;
            // TODO add more details here
            control.createActionControl(action, li);
            var desc = document.createElement('span');
            desc.className = 'description';
            desc.textContent = action.id;
            li.appendChild(desc);
            this.actionList.appendChild(li);
        });

        ///////////////////////

        element.appendChild(dl);

        this.tabControl = new TabControl(dl);
    }

    public unload()
    {
        delete this.settingList;
        delete this.actionList;
        delete this.filter;
        delete this.tabControl;
    }

    private setFilterText(filterText: string)
    {
        _.each(this.settingList.children, (li: HTMLLIElement) =>
        {
            var path = li.dataset['path'];
            li.style.display = !filterText.length || path.indexOf(filterText) !== -1 ? 'block' : 'none';
        });

        _.each(this.actionList.children, (li: HTMLLIElement) =>
        {
            var path = li.dataset['path'];
            li.style.display = !(!filterText.length || path.indexOf(filterText) !== -1) ? 'none' : 'block';
        });
    }
}

export = ConfigModule;
