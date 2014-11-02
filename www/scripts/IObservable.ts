/**
 * @author Drew Noakes https://drewnoakes.com
 */

import ICloseable = require('ICloseable');

interface IObservable
{
    getValue(): any;

    setValue(value: any): void;

    track(callback: (value: any, oldValue?: any) => void): ICloseable;
}

export = IObservable;
