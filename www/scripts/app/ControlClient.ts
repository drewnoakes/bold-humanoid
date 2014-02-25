/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />

import DataProxy = require('DataProxy');
import constants = require('constants');
import Action = require('Action');
import Setting = require('Setting');

var actionsCallbacks = [],
    settingsCallbacks = [];

var actions,
    settings,
    actionsJson,
    settingsJson,
    subscription;


var onControlData = (data: any) =>
{
    switch (data.type) {
        case "sync":
        {
            console.log('Received control data:', data);

            actionsJson = data.actions;
            settingsJson = data.settings;

            actions = _.map(data.actions, actionData => new Action(actionData));
            settings = _.map(data.settings, settingData => new Setting(settingData));

            // Raise any queued callbacks
            _.each(actionsCallbacks, callback => callback());
            _.each(settingsCallbacks, callback => callback());

            actionsCallbacks = [];
            settingsCallbacks = [];

            break;
        }
        case "update":
        {
            console.log('updating setting value', data.path, data.value);

            console.assert(settings);

            var setting = ControlClient.getSetting(data.path);

            setting.__setValue(data.value);

            // Update cached settingsJson object
            var obj = <any>_.find(settingsJson, o => o.path === data.path);
            if (obj)
                obj.value = data.value;
            else
                console.error('No setting known with path', data.path);

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
        subscription = DataProxy.subscribe(
            constants.protocols.control,
            {
                json: true,
                onmessage: _.bind(onControlData, this),
                onerror: onerror
            }
        );
    }

    public static getSetting(path)
    {
        return _.find(settings, setting => setting.path === path);
    }

    public static withSetting(path, callback)
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
            var matches = _.filter(settings, setting => setting.path.indexOf(pathPrefix) === 0);
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

    public static withAction(id, callback)
    {
        var findAction = () =>
        {
            var match = _.find(actions, action => action.id === id);
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

    public static withActions(idPrefix, callback)
    {
        var findActions = () =>
        {
            var matches = _.filter(actions, action => action.id.indexOf(idPrefix) === 0);
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

    public static send(message)
    {
        // { "type": "action", "id": "some.action" }
        // { "type": "setting", "path": "some.setting", "value": 1234 }

        console.log('Sending control message', message);

        subscription.send(JSON.stringify(message));
    }

    public static getConfigText(matching)
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
