/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import IObservable = require('IObservable');
import ICloseable = require('ICloseable');

class Select implements ICloseable
{
    private static nextIdCounter: number = 0;

    public element: HTMLSelectElement;
    private closeable: ICloseable;

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

        this.closeable = observable.track(value =>
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

    public close(): void
    {
        this.closeable.close();
        delete this.closeable;
    }
}

export = Select;
