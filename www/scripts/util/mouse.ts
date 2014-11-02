/**
 * @author Drew Noakes http://drewnoakes.com
 */

export function polyfill(e: MouseEvent)
{
    if (typeof(e.offsetX) !== 'undefined' && typeof(e.offsetY) !== 'undefined')
        return;

    var target = <HTMLElement>(e.target || e.srcElement),
        rect = target.getBoundingClientRect();

    e.offsetX = e.clientX - rect.left;
    e.offsetY = e.clientY - rect.top;
}