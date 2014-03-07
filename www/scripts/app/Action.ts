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
        if (!args)
          args = {};
        args.type = "action";
        args.id = this.id;

        require('control').send({type: "action", id: this.id});
    }
}

export = Action;
