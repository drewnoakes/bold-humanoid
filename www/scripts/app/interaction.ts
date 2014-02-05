/**
 * @author Drew Noakes
 */

import geometry = require('./Geometry');

export enum MouseButton
{
    Left = 0,
    Middle = 1,
    Right = 2
}

export interface IDragEvent
{
    isStart: boolean;
    isEnd: boolean;
    totalDeltaX: number;
    totalDeltaY: number;
    lastDeltaX: number;
    lastDeltaY: number;
    startOffsetX: number;
    startOffsetY: number;
    lastOffsetX: number;
    lastOffsetY: number;
    button: MouseButton;
}

export class Dragger
{
    private isDragging: boolean;
    private startScreenPoint: geometry.IPoint;
    private lastScreenPoint: geometry.IPoint;
    private startEvent: MouseEvent;
    private isStart: boolean;

    constructor(private element: Element, private ondrag: (distance: IDragEvent) => void)
    {
        this.element.addEventListener('mousedown', this.onmousedown.bind(this));
        this.onmouseup = this.onmouseup.bind(this);
        this.onmousemove = this.onmousemove.bind(this);
    }

    private onmousedown(e: MouseEvent)
    {
        this.fixEvent(e);
        this.lastScreenPoint = { x: e.screenX, y: e.screenY };
        this.startScreenPoint = { x: e.screenX, y: e.screenY };
        this.startEvent = e;
        this.isDragging = true;
        this.isStart = true;
        this.element.ownerDocument.addEventListener('mouseup', this.onmouseup);
        this.element.ownerDocument.addEventListener('mousemove', this.onmousemove);
    }

    private onmouseup(e: MouseEvent)
    {
        this.fixEvent(e);
        this.isDragging = false;
        this.element.ownerDocument.removeEventListener('mouseup', this.onmouseup);
        this.element.ownerDocument.removeEventListener('mousemove', this.onmousemove);
        this.raiseEvent(e, true);
    }

    private raiseEvent(e: MouseEvent, isEnd: boolean)
    {
        var totalDeltaX = e.screenX - this.startScreenPoint.x;
        var totalDeltaY = e.screenY - this.startScreenPoint.y;
        this.ondrag({
            isStart: this.isStart,
            isEnd: isEnd,
            totalDeltaX: totalDeltaX,
            totalDeltaY: totalDeltaY,
            lastDeltaX: e.screenX - this.lastScreenPoint.x,
            lastDeltaY: e.screenY - this.lastScreenPoint.y,
            startOffsetX: this.startEvent.offsetX,
            startOffsetY: this.startEvent.offsetY,
            lastOffsetX: this.startEvent.offsetX + totalDeltaX,
            lastOffsetY: this.startEvent.offsetY + totalDeltaY,
            button: this.startEvent.button
        });
        this.lastScreenPoint = { x: e.screenX, y: e.screenY };
    }

    private onmousemove(e: MouseEvent)
    {
        e.preventDefault();

        this.fixEvent(e);
        this.raiseEvent(e, false);
        this.isStart = false;
    }

    private fixEvent(e)
    {
        if (!e.hasOwnProperty('offsetX')) {
            var curleft = 0,
                curtop = 0;
            if (e.offsetParent) {
                var obj = e;
                do {
                    curleft += obj.offsetLeft;
                    curtop += obj.offsetTop;
                } while (obj = obj.offsetParent);
            }
            e.offsetX = e.layerX - curleft;
            e.offsetY = e.layerY - curtop;
        }
        return e;
    }
}