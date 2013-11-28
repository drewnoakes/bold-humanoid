/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        'use strict';

        var parser = new DOMParser();

        var DOMTemplate = function(templateId, contentType)
        {
            this.contentType = contentType || "text/html";
            var templateElement = document.getElementById(templateId);
            console.assert(templateElement);
            var templateText = templateElement.textContent;
            console.assert(templateText);
            this.template = Handlebars.compile(templateText);
        };

        DOMTemplate.prototype.create = function (data)
        {
            var obj = parser.parseFromString(this.template(data), this.contentType);
            console.assert(obj);

            if (this.contentType === "text/html")
            {
                console.assert(obj.body);
                console.assert(obj.body.childNodes.length);
                return obj.body.firstChild;
            }
            else
            {
                return obj;
            }
        };

        return DOMTemplate;
    }
);