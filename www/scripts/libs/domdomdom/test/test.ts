/**
 * @author Drew Noakes https://drewnoakes.com
 * @date 19 June 2014
 */

import dom = require('../domdomdom');

function expectEqual(expected: any, actual: any)
{
    if (expected === null && actual !== null)
        throw new Error("Expected null but got: " + JSON.stringify(actual));

    if (expected === undefined && actual !== undefined)
        throw new Error("Expected null but got: " + JSON.stringify(actual));

    if (typeof(expected) !== typeof(actual))
    {
        console.assert(typeof(expected) === typeof(actual), "Values should have the same type.\nExpected: " + typeof(expected) + "\nActual:   " + typeof(actual))
        return;
    }

    switch (typeof(expected))
    {
        case "number":
        case "string":
        case "boolean":
            if (expected !== actual)
                throw new Error("Expected '" + expected + "' but got '" + actual + "'");
            break;

        case "function":
            throw new Error("Cannot compare functions for equality.");

        case "object":
            if (expected instanceof Element)
            {
                if (expected !== actual)
                    throw new Error("Expected elements to be the same: '" + expected + "' but got '" + actual + "'");
                break;
            }

            if (expected instanceof RegExp)
            {
                expectEqual(expected.source, actual.source);
                break;
            }

            var expectedStr = JSON.stringify(expected);
            var actualStr = JSON.stringify(actual);

            if (expectedStr !== actualStr)
                throw new Error("Objects not equal.\nExpected: " + expectedStr + "\nActual:   " + actualStr);

            break;

        default:
            console.error("Unable to compare values of type: " + typeof(expected))
    }
}

function test()
{
    console.log("Running test...");

    var div1 = dom("div");
    expectEqual("DIV", div1.tagName);
    expectEqual("", div1.className);

    var ul1 = dom('ul', dom('li'));
    expectEqual(1, ul1.childElementCount);
    expectEqual('LI', ul1.children[0].tagName);

    var div2 = dom("div.foo");
    expectEqual("DIV", div2.tagName);
    expectEqual("foo", div2.className);

//    TODO support element IDs too
//
//    var div3 = dom("div#foo");
//    expectEqual("DIV", div3.tagName);
//    expectEqual("", div3.className);
//    expectEqual("foo", div3.id);
//
//    var div4 = dom("div.abc#foo");
//    expectEqual("DIV", div4.tagName);
//    expectEqual("abc", div4.className);
//    expectEqual("foo", div4.id);
//
//    var div5 = dom("div#foo.abc");
//    expectEqual("DIV", div5.tagName);
//    expectEqual("abc", div5.className);
//    expectEqual("foo", div5.id);

    var div6 = dom("div.foo", {background: "red"});
    expectEqual("DIV", div6.tagName);
    expectEqual("foo", div6.className);
    expectEqual("red", div6.style.background);

    var a1 = dom("a", "hello");
    expectEqual(true, a1 instanceof HTMLAnchorElement);
    expectEqual("A", a1.tagName);
    expectEqual("hello", a1.textContent);

    var a2 = document.createElement('a');
    expectEqual(a2, dom(a2));

    var a3 = document.createElement('a');
    dom(a3, "hello");
    expectEqual("hello", a3.textContent);

    document.body.style.backgroundColor = "green";
    document.body.textContent = "All tests passed!!!";
}

try
{
    test();
}
catch (e)
{
    document.body.style.backgroundColor = "red";
    document.body.textContent = "Test failed\n" + e;
}
