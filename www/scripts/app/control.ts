/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import Action = require('Action');
import Checkbox = require('controls/Checkbox');
import Closeable = require('util/Closeable');
import color = require('color');
import constants = require('constants');
import data = require('data');
import HsvRangeEditor = require('controls/HsvRangeEditor');
import scripts = require('scripts');
import Select = require('controls/Select');
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
    actions?: {id: string; label: string; hasArguments:boolean}[];
    settings?: any[];
    fsms: FSM[];
    motionScripts: scripts.MotionScript[];
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
    return _.find<Setting>(allSettings, setting => setting.path === path);
}

export function getAllSettings()
{
    return allSettings;
}

export function getFsmDescriptions(): FSM[]
{
    return allFSMs;
}

export function getFSM(name: string): FSM
{
    return _.find<FSM>(allFSMs, fsm => fsm.name === name);
}

export function getAction(id: string): Action
{
    return _.find<Action>(allActions, action => action.id === id);
}

export function getAllActions()
{
    return allActions;
}

function withAction(id: string, callback: (action:Action)=>void)
{
    var findAction = () =>
    {
        var match = getAction(id);
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
        var matches = _.filter<Action>(allActions, action => action.id.indexOf(idPrefix) === 0);
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
    // { "type": "setting", "id": "some.setting", "args": 1234 }

    console.log('Sending control message', message);

    subscription.send(JSON.stringify(message));
}

export function getActionText(matching?: string)
{
    if (!actionsJson)
        return '';

    var response = actionsJson;

    if (typeof(matching) === 'string' && matching.length !== 0) {
        response = _.filter<Action>(actionsJson, action => action.id.indexOf(matching) !== -1);
    }

    return JSON.stringify(response, null, 4);
}

export function buildAction(id: string, target: Element)
{
    console.assert(!!id && !!target);

    withAction(id, action => createActionControl(action, target));
}

export function buildActions(idPrefix: string, target: Element)
{
    console.assert(!!idPrefix && !!target);

    withActions(idPrefix, actions =>
    {
        _.each(actions, action =>
        {
            if (!action.hasArguments)
                buildAction(action.id, target);
        });
    });
}

export function buildSetting(path: string, container: Element, closeable: Closeable)
{
    console.assert(!!path && !!container);

    withSetting(path, setting => createSettingControl(setting, container, closeable));
}

export function buildSettings(pathPrefix: string, container: Element, closeable: Closeable)
{
    console.assert(!!pathPrefix && !!container);

    withSettings(pathPrefix, settings =>
    {
        var sortedSettings = settings.sort((a, b) => (a.type == "bool") != (b.type == "bool") ? 1 : 0);
        _.each(sortedSettings, setting => createSettingControl(setting, container, closeable))
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

            // TODO merge new actions with old actions, updating old values as needed, so that reconnect works properly

            allActions = _.map(syncData.actions, actionData => new Action(actionData));
            allSettings = _.map(syncData.settings, settingData => new Setting(settingData));
            allFSMs = syncData.fsms;
            scripts.allMotionScripts = syncData.motionScripts;

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
            var obj = _.find<ControlUpdateData>(settingsJson, o => o.path === updateData.path);
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

export function withSetting(path: string, callback: (setting:Setting)=>void)
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
        var matches = _.filter<Setting>(allSettings, setting => setting.path.indexOf(pathPrefix) === 0);
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

export function createActionControl(action: Action, target: Element)
{
    console.assert(!action.hasArguments);
    console.assert(!!target);

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

    if (!button.textContent)
        button.innerHTML = action.label;

    button.addEventListener('click', () => action.activate());
}

export function createSettingControl(setting: Setting, container: Element, closeable: Closeable, hideLabel?: boolean)
{
    if (!setting || setting.isReadOnly)
        return;

    if (hideLabel == null)
        hideLabel = false;

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
            if (!hideLabel)
            {
                var label = document.createElement('label');
                label.textContent = setting.getDescription();
                label.htmlFor = checkboxName;
                wrapper.appendChild(label);
            }
            closeable.add(setting.track(value => checkbox.checked = value));
            break;
        }
        case "enum":
        {
            if (!hideLabel)
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);
            }

            var select = new Select(<any>setting, setting.enumValues);
            closeable.add(select);
            wrapper.appendChild(select.element);
            break;
        }
        case "int":
        {
            if (!hideLabel)
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);
            }

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
            if (!hideLabel)
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);
            }

            input = document.createElement('input');
            input.type = 'number';
            input.value = setting.value;
            if (typeof(setting.min) !== 'undefined')
                input.min = setting.min.toString();
            if (typeof(setting.max) !== 'undefined')
                input.max = setting.max.toString();
            wrapper.appendChild(input);

            input.addEventListener('change', () => setting.setValue(parseFloat(input.value)));
            closeable.add(setting.track(value => input.value = value));
            break;
        }
        case "double-range":
        {
            if (!hideLabel)
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);
            }

            console.assert(setting.value instanceof Array && setting.value.length === 2);

            var minInput = document.createElement('input');
            minInput.type = 'number';
            minInput.value = setting.value[0];
            if (typeof(setting.min) !== 'undefined')
                minInput.min = setting.min.toString();
            if (typeof(setting.max) !== 'undefined')
                minInput.max = setting.max.toString();
            wrapper.appendChild(minInput);

            var maxInput = document.createElement('input');
            maxInput.type = 'number';
            maxInput.value = setting.value[1];
            if (typeof(setting.min) !== 'undefined')
                maxInput.min = setting.min.toString();
            if (typeof(setting.max) !== 'undefined')
                maxInput.max = setting.max.toString();
            wrapper.appendChild(maxInput);

            var onChange = () => setting.setValue([parseFloat(minInput.value), parseFloat(maxInput.value)]);
            minInput.addEventListener('change', onChange);
            maxInput.addEventListener('change', onChange);
            closeable.add(setting.track(value => { minInput.value = value[0]; maxInput.value = value[1]; }));
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
            if (!hideLabel)
            {
                heading = document.createElement('h3');
                heading.textContent = setting.getDescription();
                wrapper.appendChild(heading);
            }

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
