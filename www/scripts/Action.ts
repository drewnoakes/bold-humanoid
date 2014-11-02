/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/require.d.ts" />

class Action
{
    public label: string;
    public id: string;
    public hasArguments: boolean;

    constructor(actionData: {id: string; label: string; hasArguments: boolean})
    {
        this.id = actionData.id;
        this.label = actionData.label;
        this.hasArguments = actionData.hasArguments;
    }

    public activate(args?: any)
    {
        console.assert(this.hasArguments === !!args);

        var message: any = {
            type: "action",
            id: this.id
        };

        if (this.hasArguments)
            message.args = args;

        require('control').send(message);
    }
}

export = Action;
