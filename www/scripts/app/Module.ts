/**
 * @author Drew Noakes http://drewnoakes.com
 */

interface Module
{
    id: string;
    title: string;
    element: HTMLElement;

    load?:()=>void;
    unload?:()=>void;

    onResized?:(width: number, height: number)=>void;
}

export = Module;