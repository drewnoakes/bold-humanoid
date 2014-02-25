/**
 * @author Drew Noakes http://drewnoakes.com
 */

class Setting
{
    private path: string;
    private type: string;
    private isReadOnly: boolean;
    private isAdvanced: boolean;
    private defaultValue: any;
    private value: any;
    private description: string;
    private callbacks: {(value:any):void}[];
    private enumValues: any[];
    private min: number;
    private max: number;

    constructor(settingData)
    {
        this.path = settingData.path;
        this.type = settingData.type;
        this.isReadOnly = settingData.readonly;
        this.isAdvanced = settingData.advanced;
        this.defaultValue = settingData.default;
        this.value = settingData.value;
        this.description = settingData.description;
        this.callbacks = [];

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

    public setValue(value)
    {
        require('ControlClient').send({type: "setting", path: this.path, value: value});
    }

    private __setValue(value)
    {
        this.value = value;
        _.each(this.callbacks, callback => callback(value));
    }

    public track(callback)
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
