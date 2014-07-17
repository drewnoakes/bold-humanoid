/**
 * @author Drew Noakes https://drewnoakes.com
 */

interface IObservable
{
    getValue(): any;

    setValue(value: any): void;

    track(callback: (value: any, oldValue?: any) => void);
}

export = IObservable;
