/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts" />
/// <reference path="../libs/require.d.ts" />

import IObservable = require('IObservable');
import ICloseable = require('ICloseable');

class Setting implements IObservable
{
    public path: string;
    public type: string;
    public isReadOnly: boolean;
    public defaultValue: any;
    public value: any;
    public description: string;
    public enumValues: {text:string;value:string}[];
    public min: number;
    public max: number;

    private callbacks: {(value:any):void}[] = [];

    constructor(settingData)
    {
        this.path = settingData.path;
        this.type = settingData.type;
        this.isReadOnly = settingData.readonly;
        this.defaultValue = settingData.default;
        this.value = settingData.value;
        this.description = settingData.description;

        if (this.type === "enum") {
            this.enumValues = settingData.values;
        }
        else if (this.type === "int" || this.type === "double") {
            this.min = settingData.min;
            this.max = settingData.max;
        }
    }

    public getDescription()
    {
        if (this.description)
            return this.description;

        var i = this.path.lastIndexOf('.');
        var desc = this.path.substr(i + 1).replace(/-/g, ' ');
        return desc.charAt(0).toUpperCase() + desc.slice(1)
    }

    public getValue()
    {
        return this.value;
    }

    public setValue(value)
    {
        require('control').send({type: "setting", path: this.path, value: value});
    }

    public __setValue(value)
    {
        this.value = value;
        _.each(this.callbacks, callback => callback(value));
    }

    public track(callback): ICloseable
    {
        callback(this.value);
        this.callbacks.push(callback);

        return {
            close: () =>
            {
                var i = this.callbacks.indexOf(callback);
                if (i !== -1)
                    this.callbacks.splice(i, 1);
            }
        };
    }
}

export = Setting;
