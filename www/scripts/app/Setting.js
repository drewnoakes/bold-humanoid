/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        'use strict';

        var Setting = function(settingData)
        {
            this.path = settingData.path;
            this.type = settingData.type;
            this.isReadOnly = settingData.readonly;
            this.isAdvanced = settingData.advanced;
            this.defaultValue = settingData.default;
            this.value = settingData.value;
            this.description = settingData.description;
            this.callbacks = [];

            if (this.type === "enum")
            {
                this.enumValues = settingData.values;
            }
            else if (this.type === "int" || this.type === "double")
            {
                this.min = settingData.min;
                this.max = settingData.max;
            }
        };

        Setting.prototype.getDescription = function ()
        {
            if (this.description)
                return this.description;

            var i = this.path.lastIndexOf('.');
            var desc = this.path.substr(i + 1).replace('-', ' ');
            return desc.charAt(0).toUpperCase() + desc.slice(1)
        };

        Setting.prototype.setValue = function (value)
        {
            require('ControlClient').send({type: "setting", path: this.path, value: value});
        };

        Setting.prototype.__setValue = function (value)
        {
            this.value = value;
            _.each(this.callbacks, function(callback) { callback(value); });
        };

        Setting.prototype.track = function (callback)
        {
            callback(this.value);
            this.callbacks.push(callback);

            return {
                close: function()
                {
                    var i = this.callbacks.indexOf(callback);
                    if (i !== -1)
                        this.callbacks.splice(i, 1);
                }.bind(this)
            };
        };

        return Setting;
    }
);