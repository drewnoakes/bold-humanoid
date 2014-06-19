/**
 * @author Drew Noakes https://drewnoakes.com
 */

export function choose(...items: any[])
{
    var index = Math.floor(Math.random() * items.length);
    return items[index];
}

export function arrayToHsla(hsl: number[], override?: { s?: number; l?: number })
{
    var s = override && override.s || hsl[1];
    var l = override && override.l || hsl[2];
    return 'hsla(' + hsl[0] + ',' + (s * 100) + '%,' + (l * 100) + '%,1)';
}

export function clearChildren(el: Element)
{
    while (el.hasChildNodes()) {
        el.removeChild(el.lastChild);
    }
}

export function clone<T>(obj: T): T
{
    return <T>JSON.parse(JSON.stringify(obj));
}

export function getPosition(element: HTMLElement)
{
    // TODO rename to explain exactly what position this gets :)

    var xPosition = 0;
    var yPosition = 0;

    while (element)
    {
        xPosition += (element.offsetLeft - element.scrollLeft + element.clientLeft);
        yPosition += (element.offsetTop - element.scrollTop + element.clientTop);
        element = <HTMLElement>element.offsetParent;
    }

    return { x: xPosition, y: yPosition };
}

function binarySearchComparator<T>(array: T[], comparator: (item: T)=>number)
{
    var minIndex = 0;
    var maxIndex = array.length - 1;

    while (minIndex <= maxIndex)
    {
        var currentIndex = (minIndex + maxIndex) / 2 | 0;
        var currentElement = array[currentIndex];

        var comparison = comparator(currentElement);

        if (comparison < 0)
            minIndex = currentIndex + 1;
        else if (comparison > 0)
            maxIndex = currentIndex - 1;
        else
            return currentIndex;
    }

    return -1;
}

//
// FULL SCREEN SUPPORT
//

export function isFullScreen()
{
    var doc: any = document;

    return (doc.fullScreenElement && doc.fullScreenElement !== null)
        || doc.mozFullScreen
        || doc.webkitIsFullScreen;
}

export function requestFullScreen(element: Element)
{
    var elem: any = element;

    if (elem.requestFullscreen)
        elem.requestFullscreen();
    else if (elem.msRequestFullscreen)
        elem.msRequestFullscreen();
    else if (elem.mozRequestFullScreen)
        elem.mozRequestFullScreen();
    else if (elem.webkitRequestFullscreen)
        elem.webkitRequestFullscreen();
}

export function cancelFullScreen()
{
    var doc: any = document;

    if (doc.exitFullscreen)
        doc.exitFullscreen();
    else if (doc.msExitFullscreen)
        doc.msExitFullscreen();
    else if (doc.mozCancelFullScreen)
        doc.mozCancelFullScreen();
    else if (doc.webkitExitFullscreen)
        doc.webkitExitFullscreen();
}

export function toggleFullScreen(element?: Element)
{
    element = element || document.documentElement;

    if (isFullScreen())
        cancelFullScreen();
    else
        requestFullScreen(element);
}

