/**
 * @author Drew Noakes http://drewnoakes.com
 */

import HsvRange = require('HsvRange');
import color = require('color');

var channels = ['hue', 'sat', 'val'];

interface IRowSpec
{
    minCol: HTMLSpanElement;
    maxCol: HTMLSpanElement;
    minInput: HTMLInputElement;
    maxInput: HTMLInputElement;
}

class HsvRangeEditor
{
    public element: HTMLDivElement;
    private rows: {hue?:IRowSpec; sat?:IRowSpec; val?:IRowSpec;};
    private updatesSuspended: boolean = false;
    private callback: (any) => void;

    constructor(title: string)
    {
        this.element = document.createElement('div');
        this.element.className = 'hsv-range-editor';

        var heading = document.createElement('h3');
        heading.textContent = title;
        this.element.appendChild(heading);

        this.rows = {};
        this.addRow('hue');
        this.addRow('sat');
        this.addRow('val');
    }

    private addRow(channelName: string)
    {
        var row = document.createElement('div');
        row.className = 'channel-row ' + channelName;

        var minCol = document.createElement('span'),
            maxCol = document.createElement('span'),
            label = document.createElement('span'),
            minInput = document.createElement('input'),
            maxInput = document.createElement('input');

        label.textContent = channelName[0].toUpperCase();

        minInput.type = 'text';
        maxInput.type = 'text';

        minInput.addEventListener('change', this.onTextChange.bind(this));
        maxInput.addEventListener('change', this.onTextChange.bind(this));

        row.appendChild(minCol);
        row.appendChild(minInput);
        row.appendChild(label);
        row.appendChild(maxInput);
        row.appendChild(maxCol);

        this.rows[channelName] = {
            minCol: minCol,
            maxCol: maxCol,
            minInput: minInput,
            maxInput: maxInput
        };

        this.element.appendChild(row);
    }

    private onTextChange()
    {
        if (this.updatesSuspended || !this.callback)
            return;

        var val = {
            hue: [parseInt(this.rows.hue.minInput.value), parseInt(this.rows.hue.maxInput.value)],
            sat: [parseInt(this.rows.sat.minInput.value), parseInt(this.rows.sat.maxInput.value)],
            val: [parseInt(this.rows.val.minInput.value), parseInt(this.rows.val.maxInput.value)]
        };

        this.callback(val);
    }

    public onChange(callback)
    {
        this.callback = callback;
    }

    public setValue(value)
    {
        this.updatesSuspended = true;

        _.each(channels, channel =>
        {
            this.rows[channel].minInput.value = value[channel][0];
            this.rows[channel].maxInput.value = value[channel][1];
        });

        var hAvg = (value.hue[0] + value.hue[1]) / 2,
            h = value.hue[0] < value.hue[1] ? hAvg : (hAvg + (255/2)) % 255,
            s = (value.sat[0] + value.sat[1]) / 2,
            v = (value.val[0] + value.val[1]) / 2;

        h /= 255;
        s /= 255;
        v /= 255;

        this.rows.hue.minCol.style.backgroundColor = new color.Hsv(value.hue[0]/255, s, v).toString();
        this.rows.hue.maxCol.style.backgroundColor = new color.Hsv(value.hue[1]/255, s, v).toString();
        this.rows.sat.minCol.style.backgroundColor = new color.Hsv(h, value.sat[0]/255, v).toString();
        this.rows.sat.maxCol.style.backgroundColor = new color.Hsv(h, value.sat[1]/255, v).toString();
        this.rows.val.minCol.style.backgroundColor = new color.Hsv(h, s, value.val[0]/255).toString();
        this.rows.val.maxCol.style.backgroundColor = new color.Hsv(h, s, value.val[1]/255).toString();

        var h3: HTMLHeadingElement = <HTMLHeadingElement>this.element.querySelector('h3');
        h3.style.backgroundColor = HsvRange.calculateColour(value);

        this.updatesSuspended = false;
    }
}

export = HsvRangeEditor;
