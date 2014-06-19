/**
 * @author Drew Noakes https://drewnoakes.com
 * @date 19 June 2014
 */

function dom(...values: any[]): HTMLElement
{
    if (values.length === 0)
        return undefined;

    var element: HTMLElement;

    for (var i = 0; i < values.length; i++)
    {
        var val = values[i],
            valType = typeof(val);

        if (valType === "object")
        {
            if (val instanceof Element)
            {
                if (i === 0)
                {
                    element = val;
                }
                else
                {
                    console.assert(!!element);
                    element.appendChild(val);
                }
            }
            else
            {
                console.assert(!!element);
                var props = Object.getOwnPropertyNames(val);
                for (var i = 0; i < props.length; i++)
                {
                    if (element.style.hasOwnProperty(props[i]))
                        element.style[props[i]] = val[props[i]];
                }
            }
        }
        else if (valType === "string")
        {
            if (i === 0)
            {
                // String in the first position is a tag or tag/class
                var dotIndex = val.indexOf('.'),
                    hashIndex = val.indexOf('#');

                if (dotIndex === -1)
                {
                    // string is just a tag name
                    element = document.createElement(val);
                    console.log(dotIndex, element.tagName, element.className);
                }
                else
                {
                    // String is a tag and a class
                    console.assert(dotIndex !== 0);
                    element = document.createElement(val.substring(0, dotIndex));
                    element.className = val.substring(dotIndex + 1);
                    console.log(dotIndex, element.tagName, element.className);
                }
            }
            else
            {
                console.log("B", val);
                console.assert(element != null);
                element.textContent = val;
            }
        }
        else
        {
            console.log("Unexpected value type: " + valType);
        }
    }

    return element;
}

export = dom
