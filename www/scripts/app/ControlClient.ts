/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import data = require('data');
import constants = require('constants');
import Action = require('Action');
import Setting = require('Setting');

var actionsCallbacks = [],
    settingsCallbacks = [];

var actions,
    settings,
    actionsJson,
    settingsJson,
    fsmJson,
    subscription;

interface FSMState
{
    id: string;
}

interface FSMTransition
{
    label: string;
    from: string;
    to: string;
}

interface FSMWildcardTransition
{
    label: string;
    to: string;
}

interface FSM
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

/*
// TODO change this from a static class to a module, export this interface and use it on the API
// { "type": "action", "id": "some.action" }
// { "type": "setting", "path": "some.setting", "value": 1234 }
interface ControlMessage
{
    type: string;
    id?: string;
    path?: string;
    value?: any;
}
*/

var onControlData = (data: ControlData) =>
{
    switch (data.type) {
        case "sync":
        {
            var syncData = <ControlSyncData>data;

            console.log('Received control data:', syncData);

            actionsJson = syncData.actions;
            settingsJson = syncData.settings;
            fsmJson = syncData.fsms;

            actions = _.map(syncData.actions, actionData => new Action(actionData));
            settings = _.map(syncData.settings, settingData => new Setting(settingData));

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
            console.assert(settings);

            var setting = ControlClient.getSetting(updateData.path);

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
};

var onSettingChangeCallbacks = [];

class ControlClient
{
    public static onSettingChange(callback)
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

    public static connect(onerror)
    {
        subscription = new data.Subscription<ControlData>(
            constants.protocols.control,
            {
                onmessage: onControlData,
                onerror: onerror
            }
        );
    }

    public static getSetting(path) : Setting
    {
        return <Setting>_.find(settings, setting => setting.path === path);
    }

    public static withSetting(path, callback: (setting:Setting)=>void)
    {
        var process = () =>
        {
            var match = ControlClient.getSetting(path);
            if (!match)
                console.error("No settings exists with path: " + path);
            callback(match);
        };

        if (settings) {
            // We have data, so provide it immediately
            process();
        }
        else {
            // No data yet, so store the callback
            settingsCallbacks.push(process);
        }
    }

    public static withSettings(pathPrefix, callback: (settings:Setting[])=>void)
    {
        var findSettings = () =>
        {
            var matches = <Setting[]>_.filter(settings, setting => setting.path.indexOf(pathPrefix) === 0);
            if (matches.length === 0)
                console.error("No settings exist with path prefix: " + pathPrefix);
            callback(matches);
        };

        if (settings) {
            // We have data, so provide it immediately
            findSettings();
        }
        else {
            // No data yet, so store the callback
            settingsCallbacks.push(findSettings);
        }
    }

    public static withAction(id, callback: (action:Action)=>void)
    {
        var findAction = () =>
        {
            var match = <Action>_.find(actions, action => action.id === id);
            if (!match)
                console.error("No action exist with ID: " + id);
            callback(match);
        };

        if (actions) {
            // We have data, so provide it immediately
            findAction();
        }
        else {
            // No data yet, so store the callback
            actionsCallbacks.push(findAction);
        }
    }

    public static withActions(idPrefix, callback: (actions:Action[])=>void)
    {
        var findActions = () =>
        {
            var matches = <Action[]>_.filter(actions, action => action.id.indexOf(idPrefix) === 0);
            if (matches.length === 0)
                console.error("No actions exist with ID prefix: " + idPrefix);
            callback(matches);
        };

        if (actions) {
            // We have data, so provide it immediately
            findActions();
        }
        else {
            // No data yet, so store the callback
            actionsCallbacks.push(findActions);
        }
    }

    public static send(message: any)
    {
        // { "type": "action", "id": "some.action" }
        // { "type": "setting", "path": "some.setting", "value": 1234 }

        console.log('Sending control message', message);

        subscription.send(JSON.stringify(message));
    }

    public static getConfigText(matching?: string)
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
}

export = ControlClient;
