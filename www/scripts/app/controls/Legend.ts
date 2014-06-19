/**
 * @author Drew Noakes https://drewnoakes.com
 */

import dom = require("../../libs/domdomdom/domdomdom");

class Legend
{
    public element: HTMLElement;

    constructor(items: {colour: string; name: string}[])
    {
        this.element = dom('ul.legend');

        for (var i = 0; i < items.length; i++)
            this.addItem(items[i].name,  items[i].colour);
    }

    private addItem(name: string, colour: string)
    {
        dom(this.element,
            dom("li.tile",
                dom("div.tile", {background: colour}),
                dom("div.label", name)
            )
        );
    }
}

export = Legend
