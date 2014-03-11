/**
 * @author Drew Noakes https://drewnoakes.com
 */

import util = require('util');

export class ColorPicker
{
    public element: HTMLInputElement;

    constructor(trackable: util.Trackable<string>)
    {
        this.element = <HTMLInputElement>document.createElement('input');
        this.element.type = 'color';
        this.element.addEventListener('change', e => trackable.setValue(this.element.value));

        trackable.track(color => this.element.value = color);
    }
}
