// Type definitions for Hammer.js 1.0.10
// Project: http://eightmedia.github.com/hammer.js/
// Definitions by: Boris Yankov <https://github.com/borisyankov/>
// Definitions: https://github.com/borisyankov/DefinitelyTyped

declare var Hammer: HammerStatic;

interface HammerStatic
{
    (element: any, options?: HammerOptions): HammerInstance;

    VERSION: number;
    HAS_POINTEREVENTS: boolean;
    HAS_TOUCHEVENTS: boolean;
    UPDATE_VELOCITY_INTERVAL: number;
    POINTER_MOUSE: HammerPointerType;
    POINTER_TOUCH: HammerPointerType;
    POINTER_PEN: HammerPointerType;

    DIRECTION_UP: HammerDirectionType;
    DIRECTION_DOWN: HammerDirectionType;
    DIRECTION_LEFT: HammerDirectionType;
    DIRECTION_RIGH: HammerDirectionType;

    EVENT_START: HammerTouchEventState;
    EVENT_MOVE: HammerTouchEventState;
    EVENT_END: HammerTouchEventState;

    plugins: any;
    gestures: any;
    READY: boolean;

}

declare class HammerInstance
{
    constructor(element: any, options?: HammerOptions);

    on(gesture: string, handler: (event: HammerEvent) => void): HammerInstance;

    off(gesture: string, handler: (event: HammerEvent) => void): HammerInstance;

    enable(toggle: boolean): HammerInstance;

    // You shouldn't use this, this is an internally method use by the gestures. Only use it when you know what you're doing! You can read the sourcecode about how to use this.
    trigger(gesture: string, eventData: HammerGestureEventData): HammerInstance;
}

// Gesture Options : https://github.com/EightMedia/hammer.js/wiki/Getting-Started#gesture-options
interface HammerOptions
{
    behavior?: {
        contentZooming?: string;
        tapHighlightColor?: string;
        touchAction?: string;
        touchCallout?: string;
        userDrag?: string;
        userSelect?: string;
    };
    doubleTapDistance?: number;
    doubleTapInterval?: number;
    drag?: boolean;
    dragBlockHorizontal?: boolean;
    dragBlockVertical?: boolean;
    dragDistanceCorrection?: boolean;
    dragLockMinDistance?: number;
    dragLockToAxis?: boolean;
    dragMaxTouches?: number;
    dragMinDistance?: number;
    gesture?: boolean;
    hold?: boolean;
    holdThreshold?: number;
    holdTimeout?: number;
    preventDefault?: boolean;
    preventMouse?: boolean;
    release?: boolean;
    showTouches?: boolean;
    swipe?: boolean;
    swipeMaxTouches?: number;
    swipeMinTouches?: number;
    swipeVelocityX?: number;
    swipeVelocityY?: number;
    tap?: boolean;
    tapAlways?: boolean;
    tapMaxDistance?: number;
    tapMaxTime?: number;
    touch?: boolean;
    transform?: boolean;
    transformMinRotation?: number;
    transformMinScale?: number;
}

interface HammerGestureEventData
{
    timestamp: number;
    target: HTMLElement;
    touches: HammerPoint[];
    pointerType: HammerPointerType;
    center: HammerPoint;
    deltaTime: number;
    deltaX: number;
    deltaY: number;
    velocityX: number;
    velocityY: number;
    angle: number;
    interimAngle: number;
    direction: HammerDirectionType;
    interimDirection: HammerDirectionType;
    distance: number;
    scale: number;
    rotation: number;
    eventType: HammerTouchEventState;
    srcEvent: any;
    startEvent: any;

    stopPropagation(): void;
    preventDefault(): void;
    stopDetect(): void;
}

interface HammerPoint
{
    clientX: number;
    clientY: number;
    pageX: number;
    pageY: number;
}

interface HammerEvent
{
    type: string;
    gesture: HammerGestureEventData;

    stopPropagation(): void;
    preventDefault(): void;

}

declare enum HammerPointerType
{}

declare enum HammerDirectionType
{}

declare enum HammerTouchEventState
{}

