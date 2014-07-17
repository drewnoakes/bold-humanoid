/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import IObservable = require('IObservable');

class Select
{
    private static nextIdCounter: number = 0;

    public element: HTMLSelectElement;

    constructor(observable: IObservable, items: {value:any; text:string}[], id?: string)
    {
        this.element = document.createElement('select');
        this.element.id = id || 'select-number-' + (Select.nextIdCounter++);

        _.each(items, item =>
        {
            var option = document.createElement('option');
            option.text = item.text;
            option.selected = observable.getValue() == item.value;
            this.element.appendChild(option);
        });

        this.element.addEventListener('change', () => {
            observable.setValue(items[this.element.selectedIndex].value);
        });

        // TODO LEAK this closeable should be closed at some point
        observable.track(value =>
        {
            for (var i = 0; i < items.length; i++)
            {
                if (items[i].value == value)
                {
                    this.element.selectedIndex = i;
                    return;
                }
            }
            console.assert(false && !!"Observed value changed to something not specified in original items");
        });
    }
}

export = Select;
