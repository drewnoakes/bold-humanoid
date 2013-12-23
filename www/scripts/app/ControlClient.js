/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'DataProxy',
        'Protocols',
        'Action',
        'Setting'
    ],
    function (DataProxy, Protocols, Action, Setting)
    {
        var ControlClient = {};

        var actionsCallbacks = [],
            settingsCallbacks = [];

        var actions,
            settings,
            actionsJson,
            settingsJson,
            subscription;

        var onControlData = function(data)
        {
            switch (data.type)
            {
                case "sync":
                {
                    console.log('Received control data:', data);

                    actionsJson = data.actions;
                    settingsJson = data.settings;

                    actions = _.map(data.actions, function(actionData) { return new Action(actionData); });
                    settings = _.map(data.settings, function(settingData) { return new Setting(settingData); });

                    // Raise any queued callbacks
                    _.each(actionsCallbacks, function(callback) { callback(); });
                    _.each(settingsCallbacks, function(callback) { callback(); });

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
                    var obj = _.find(settingsJson, function(o) { return o.path === data.path; });
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

            _.each(onSettingChangeCallbacks, function(callback) { callback(); });
        };

        var onSettingChangeCallbacks = [];

        ControlClient.onSettingChange = function(callback)
        {
            onSettingChangeCallbacks.push(callback);

            return {
                close: function()
                {
                    var i = onSettingChangeCallbacks.indexOf(callback);
                    if (i !== -1)
                        onSettingChangeCallbacks.splice(i, 1);
                }
            }
        };

        ControlClient.connect = function(onerror)
        {
            subscription = DataProxy.subscribe(
                Protocols.control,
                {
                    json: true,
                    onmessage: _.bind(onControlData, this),
                    onerror: onerror
                }
            );
        };

        ControlClient.getSetting = function(path)
        {
            return _.find(settings, function(setting) { return setting.path === path; });
        };

        ControlClient.withSetting = function(path, callback)
        {
            var process = function()
            {
                var match = ControlClient.getSetting(path);
                if (!match)
                    console.error("No settings exists with path: " + path);
                callback(match);
            };

            if (settings)
            {
                // We have data, so provide it immediately
                process();
            }
            else
            {
                // No data yet, so store the callback
                settingsCallbacks.push(process);
            }
        };

        ControlClient.withSettings = function(pathPrefix, callback)
        {
            var findSettings = function()
            {
                var matches = _.filter(settings, function(setting) { return setting.path.indexOf(pathPrefix) === 0; });
                if (matches.length === 0)
                    console.error("No settings exist with path prefix: " + pathPrefix);
                callback(matches);
            };

            if (settings)
            {
                // We have data, so provide it immediately
                findSettings();
            }
            else
            {
                // No data yet, so store the callback
                settingsCallbacks.push(findSettings);
            }
        };

        ControlClient.withAction = function(id, callback)
        {
            var findAction = function()
            {
                var match = _.find(actions, function(action) { return action.id === id; });
                if (!match)
                    console.error("No action exist with ID: " + id);
                callback(match);
            };

            if (actions)
            {
                // We have data, so provide it immediately
                findAction();
            }
            else
            {
                // No data yet, so store the callback
                actionsCallbacks.push(findAction);
            }
        };

        ControlClient.withActions = function(idPrefix, callback)
        {
            var findActions = function()
            {
                var matches = _.filter(actions, function(action) { return action.id.indexOf(idPrefix) === 0; });
                if (matches.length === 0)
                    console.error("No actions exist with ID prefix: " + idPrefix);
                callback(matches);
            };

            if (actions)
            {
                // We have data, so provide it immediately
                findActions();
            }
            else
            {
                // No data yet, so store the callback
                actionsCallbacks.push(findActions);
            }
        };

        ControlClient.send = function(message)
        {
            // { "type": "action", "id": "some.action" }
            // { "type": "setting", "path": "some.setting", "value": 1234 }

            console.log('Sending control message', message);

            subscription.send(JSON.stringify(message));
        };

        ControlClient.getConfigText = function(matching)
        {
            // TODO allow other types of config (actions, values only, ...?)

            if (!settingsJson)
                return '';

            var response = settingsJson;

            if (typeof(matching) === 'string' && matching.length !== 0)
            {
                response = _.filter(settingsJson, function (setting)
                {
                    return setting.path.indexOf(matching) !== -1;
                });
            }

            return JSON.stringify(response, null, 4);
        };

        return ControlClient;
    }
);