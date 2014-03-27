/**
 * @author Drew Noakes https://drewnoakes.com
 */

import Trackable = require('util/Trackable');

class Checkbox
{
    private static nextIdCounter: number = 0;

    public element: HTMLDivElement;

    constructor(labelText: string, trackable: Trackable<boolean>, id?: string)
    {
        this.element = document.createElement('div');
        this.element.className = 'checkbox control';

        if (!id)
            id = 'checkbox-number-' + (Checkbox.nextIdCounter++);

        var input = <HTMLInputElement>document.createElement('input');
        input.type = 'checkbox';
        input.id = id;
        input.addEventListener('change', e => trackable.setValue(input.checked));
        this.element.appendChild(input);

        var label = <HTMLLabelElement>document.createElement('label');
        label.htmlFor = id;
        label.textContent = labelText;
        this.element.appendChild(label);

        trackable.track(checked => input.checked = checked);
    }
}

export = Checkbox;
