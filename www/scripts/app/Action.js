/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        'use strict';

        var Action = function (actionData)
        {
            this.id = actionData.id;
            this.label = actionData.label;
        };

        Action.prototype.activate = function ()
        {
            require('ControlClient').send({type: "action", id: this.id});
        };

        return Action;
    }
);