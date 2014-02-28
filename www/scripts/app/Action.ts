/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/require.d.ts" />

class Action
{
    public label: string;
    public id: string;

    constructor(actionData: {id: string; label: string;})
    {
        this.id = actionData.id;
        this.label = actionData.label;
    }

    public activate()
    {
        require('control').send({type: "action", id: this.id});
    }
}

export = Action;
