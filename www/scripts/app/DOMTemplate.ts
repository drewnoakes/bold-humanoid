/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/handlebars.d.ts" />

var parser: DOMParser = new DOMParser();

class DOMTemplate
{
    private template: (context?: any, options?: any) => string;

    constructor(private templateId: string, public contentType?: string)
    {
        this.contentType = contentType || "text/html";

        var templateElement = document.getElementById(templateId);

        if (!templateElement) {
            console.error('No element found with id', templateId);
            return;
        }

        var templateText = templateElement.textContent;

        console.assert(!!templateText);

        this.template = Handlebars.compile(templateText);
    }

    public create(data?: any): Node
    {
        var obj = parser.parseFromString(this.template(data), this.contentType);

        console.assert(!!obj);

        if (this.contentType === "text/html") {
            console.assert(!!obj.body);
            if (obj.body.children.length !== 1)
                console.error("Template '" + this.templateId + "' specifies " + obj.body.children.length + " children, where only one is allowed.");
            return obj.body.firstElementChild;
        }
        else {
            return obj;
        }
    }
}

export = DOMTemplate;
