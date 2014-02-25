/**
 * @author Drew Noakes http://drewnoakes.com
 */

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
        require('ControlClient').send({type: "action", id: this.id});
    }
}

export = Action;