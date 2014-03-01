/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import Action = require('Action');
import Closeable = require('util/Closeable');
import color = require('color');
import constants = require('constants');
import data = require('data');
import HsvRangeEditor = require('HsvRangeEditor');
import Setting = require('Setting');

export interface FSMState
{
    id: string;
}

export interface FSMTransition
{
    id: string;
    from: string;
    to: string;
}

export interface FSMWildcardTransition
{
    id: string;
    to: string;
}

export interface FSM
{
    name: string;
    start: string;
    states: FSMState[];
    transitions: FSMTransition[];
    wildcardTransitions: FSMWildcardTransition[];
}

interface ControlData
{
    /** Either 'sync' or 'update'. */
    type: string;
}

interface ControlSyncData extends ControlData
{
    actions?: {id: string; label: string;}[];
    settings?: any[];
    fsms: FSM[];
}

interface ControlUpdateData extends ControlData
{
    path?: string;
    value?: any;
}

var nextControlId = 0;

var actionsCallbacks = [],
    settingsCallbacks = [];

var allActions: Action[],
    allSettings: Setting[],
    allFSMs: FSM[],
    actionsJson,
    settingsJson,
    subscription;

var onSettingChangeCallbacks: {():void}[] = [];

// { "type": "action", "id": "some.action" }
// { "type": "setting", "path": "some.setting", "value": 1234 }
export interface ControlMessage
{
    type: string;
    id?: string;
    path?: string;
    value?: any;
}

export function getSetting(path: string): Setting
{
    return <Setting>_.find(allSettings, setting => setting.path === path);
}

export function getFsmDescriptions(): FSM[]
{
    return allFSMs;
}

export function getFSM(name: string): FSM
{
    return _.find(allFSMs, fsm => fsm.name === name);
}

function withAction(id: string, callback: (action:Action)=>void)
{
    var findAction = () =>
    {
        var match = <Action>_.find(allActions, action => action.id === id);
        if (!match)
            console.error("No action exist with ID: " + id);
        callback(match);
    };

    if (allActions) {
        // We have data, so provide it immediately
        findAction();
    }
    else {
        // No data yet, so store the callback
        actionsCallbacks.push(findAction);
    }
}

function withActions(idPrefix: string, callback: (actions:Action[])=>void)
{
    var findActions = () =>
    {
        var matches = <Action[]>_.filter(allActions, action => action.id.indexOf(idPrefix) === 0);
        if (matches.length === 0)
            console.error("No actions exist with ID prefix: " + idPrefix);
        callback(matches);
    };

    if (allActions) {
        // We have data, so provide it immediately
        findActions();
    }
    else {
        // No data yet, so store the callback
        actionsCallbacks.push(findActions);
    }
}

export function send(message: ControlMessage)
{
    // { "type": "action", "id": "some.action" }
    // { "type": "setting", "path": "some.setting", "value": 1234 }

    console.log('Sending control message', message);

    subscription.send(JSON.stringify(message));
}

export function getConfigText(matching?: string)
{
    // TODO allow other types of config (actions, values only, ...?)

    if (!settingsJson)
        return '';

    var response = settingsJson;

    if (typeof(matching) === 'string' && matching.length !== 0) {
        response = _.filter(settingsJson, setting => setting.path.indexOf(matching) !== -1);
    }

    return JSON.stringify(response, null, 4);
}

export function buildAction(id: string, target: Element)
{
    console.assert(!!id && !!target);

    var button;
    if (target instanceof HTMLButtonElement)
    {
        button = target;
    }
    else
    {
        console.assert(target instanceof Element);
        button = document.createElement('button');
        (<Element>target).appendChild(button);
    }

    withAction(id, action =>
    {
        if (!button.textContent)
            button.innerHTML = action.label;

        button.addEventListener('click', () => action.activate());
    });

    return button;
}

export function buildActions(idPrefix: string, target: Element)
{
    console.assert(!!idPrefix && !!target);

    withActions(idPrefix, actions =>
    {
        _.each(actions, action => buildAction(action.id, target));
    });
}

export function buildSetting(path: string, container: Element, closeable: Closeable)
{
    console.assert(!!path && !!container);

    withSetting(path, setting => createSetting(setting, container, closeable));
}

export function buildSettings(pathPrefix: string, container: Element, closeable: Closeable)
{
    console.assert(!!pathPrefix && !!container);

    withSettings(pathPrefix, settings =>
    {
        var sortedSettings = settings.sort((a, b) => (a.type == "bool") != (b.type == "bool") ? 1 : 0);
        _.each(sortedSettings, setting => createSetting(setting, container, closeable))
    });
}

function onControlData(data: ControlData)
{
    switch (data.type) {
        case "sync":
        {
            var syncData = <ControlSyncData>data;

            console.log('Received control data:', syncData);

            actionsJson = syncData.actions;
            settingsJson = syncData.settings;

            allActions = _.map(syncData.actions, actionData => new Action(actionData));
            allSettings = _.map(syncData.settings, settingData => new Setting(settingData));
            allFSMs = syncData.fsms;

            // Raise any queued callbacks
            _.each(actionsCallbacks, callback => callback());
            _.each(settingsCallbacks, callback => callback());

            actionsCallbacks = [];
            settingsCallbacks = [];

            break;
        }
        case "update":
        {
            var updateData = <ControlUpdateData>data;

            console.log('updating setting value', updateData.path, updateData.value);
            console.assert(!!allSettings);

            var setting = getSetting(updateData.path);

            setting.__setValue(updateData.value);

            // Update cached settingsJson object
            var obj = <any>_.find(settingsJson, o => o.path === updateData.path);
            if (obj)
                obj.value = updateData.value;
            else
                console.error('No setting known with path', updateData.path);

            break;
        }
        default:
        {
            console.error("Unsupported control data type: " + data.type);
        }
    }

    _.each(onSettingChangeCallbacks, callback => callback());
}

export function onSettingChange(callback: ()=>void)
{
    onSettingChangeCallbacks.push(callback);

    return {
        close: () =>
        {
            var i = onSettingChangeCallbacks.indexOf(callback);
            if (i !== -1)
                onSettingChangeCallbacks.splice(i, 1);
        }
    }
}

export function connect(onerror)
{
    subscription = new data.Subscription<ControlData>(
        constants.protocols.control,
        {
            onmessage: onControlData,
            onerror: onerror
        }
    );
}

function withSetting(path: string, callback: (setting:Setting)=>void)
{
    var process = () =>
    {
        var match = getSetting(path);
        if (!match)
            console.error("No settings exists with path: " + path);
        callback(match);
    };

    if (allSettings) {
        // We have data, so provide it immediately
        process();
    }
    else {
        // No data yet, so store the callback
        settingsCallbacks.push(process);
    }
}

export function withSettings(pathPrefix: string, callback: (settings:Setting[])=>void)
{
    var findSettings = () =>
    {
        var matches = <Setting[]>_.filter(allSettings, setting => setting.path.indexOf(pathPrefix) === 0);
        if (matches.length === 0)
            console.error("No settings exist with path prefix: " + pathPrefix);
        callback(matches);
    };

    if (allSettings) {
        // We have data, so provide it immediately
        findSettings();
    }
    else {
        // No data yet, so store the callback
        settingsCallbacks.push(findSettings);
    }
}

function createSetting(setting: Setting, container: Element, closeable: Closeable)
{
    if (setting.isReadOnly)
        return;

    var heading, input,
        wrapper = document.createElement('div');
    wrapper.dataset['path'] = setting.path;
    wrapper.className = 'setting control';

    switch (setting.type)
    {
        case "bool":
        {
            var checkboxName = 'checkbox' + (nextControlId++);

            var checkbox = document.createElement('input');
            checkbox.type = 'checkbox';
            checkbox.id = checkboxName;
            checkbox.addEventListener('change', () => setting.setValue(checkbox.checked));
            wrapper.appendChild(checkbox);
            var label = document.createElement('label');
            label.textContent = setting.getDescription();
            label.htmlFor = checkboxName;
            wrapper.appendChild(label);
            closeable.add(setting.track(value => checkbox.checked = value));
            break;
        }
        case "enum":
        {
            heading = document.createElement('h3');
            heading.textContent = setting.getDescription();
            wrapper.appendChild(heading);

            var select = <HTMLSelectElement>document.createElement('select');
            _.each(setting.enumValues, enumValue =>
            {
                var option = document.createElement('option');
                option.selected = setting.value === enumValue.value;
                option.text = enumValue.text;
                option.value = enumValue.value;
                select.appendChild(option);
            });
            select.addEventListener('change', () => setting.setValue(parseInt(select.options[select.selectedIndex].value)));
            closeable.add(setting.track(value =>
            {
                var option = <HTMLOptionElement>_.find(select.options, option => parseInt(option.value) === value);
                option.selected = true;
            }));
            wrapper.appendChild(select);
            break;
        }
        case "int":
        {
            heading = document.createElement('h3');
            heading.textContent = setting.getDescription();
            wrapper.appendChild(heading);

            input = document.createElement('input');
            input.type = 'number';
            input.value = setting.value;
            if (typeof(setting.min) !== 'undefined')
                input.min = setting.min;
            if (typeof(setting.max) !== 'undefined')
                input.max = setting.max;
            wrapper.appendChild(input);

            input.addEventListener('change', () => setting.setValue(parseInt(input.value)));
            closeable.add(setting.track(value => input.value = value));
            break;
        }
        case "double":
        {
            heading = document.createElement('h3');
            heading.textContent = setting.getDescription();
            wrapper.appendChild(heading);

            input = document.createElement('input');
            input.type = 'number';
            input.value = setting.value;
            if (typeof(setting.min) !== 'undefined')
                input.min = setting.min;
            if (typeof(setting.max) !== 'undefined')
                input.max = setting.max;
            wrapper.appendChild(input);

            input.addEventListener('change', () => setting.setValue(parseFloat(input.value)));
            closeable.add(setting.track(value => input.value = value));
            break;
        }
        case 'hsv-range':
        {
            var editor = new HsvRangeEditor(setting.getDescription());
            editor.onChange(value => setting.setValue(value));
            closeable.add(setting.track(value => editor.setValue(value)));
            wrapper.appendChild(editor.element);
            break;
        }
        case 'bgr-colour':
        {
            heading = document.createElement('h3');
            heading.textContent = setting.getDescription();
            wrapper.appendChild(heading);

            var colorInput = document.createElement('input');
            colorInput.type = 'color';
            colorInput.addEventListener('change', () =>
            {
                var rgb = new color.Rgb(colorInput.value);
                setting.setValue(rgb.toByteObject());
            });
            closeable.add(setting.track(value => colorInput.value = new color.Rgb(value.r/255, value.g/255, value.b/255).toString()));
            wrapper.appendChild(colorInput);
            break;
        }
        default:
        {
            console.error("Unsupported setting type", setting.type, "for", setting.path);
        }
    }

    container.appendChild(wrapper);
}
