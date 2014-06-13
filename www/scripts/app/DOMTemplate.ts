/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/handlebars.d.ts" />

var parser: DOMParser = new DOMParser();

class DOMTemplate
{
    private template: (context?: any, options?: any) => string;
    private contentType: string;

    static forId(templateId: string, contentType?: string) : DOMTemplate
    {
        var templateElement = document.getElementById(templateId);

        if (!templateElement) {
            console.error('No element found with id', templateId);
            return;
        }

        return DOMTemplate.forText(templateElement.textContent);
    }

    static forText(templateText: string, contentType?: string): DOMTemplate
    {
        console.assert(!!templateText);

        var template = new DOMTemplate();
        template.contentType = contentType;
        template.template = Handlebars.compile(templateText);
        return template;
    }

    public create(data?: any): Node
    {
        var obj = parser.parseFromString(this.template(data), this.contentType || "text/html");

        console.assert(!!obj);

        if (this.contentType == null || this.contentType === "text/html") {
            console.assert(!!obj.body);
            if (obj.body.children.length !== 1)
                console.error("Template specifies " + obj.body.children.length + " children, where only one is allowed.");
            return obj.body.firstElementChild;
        }
        else {
            return obj;
        }
    }
}

export = DOMTemplate;
