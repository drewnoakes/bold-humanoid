/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts"/>

interface ITabItem
{
    thumb: HTMLElement;
    content: HTMLDivElement;
}

class TabControl
{
    private thumbContainer: HTMLUListElement;
    private contentContainer: HTMLDivElement;
    private items: ITabItem[] = [];

    constructor(element: HTMLDListElement)
    {
        console.assert(!!element);

        // Create new elements
        var newElement = document.createElement('div');
        newElement.className = 'tab-control';
        this.contentContainer = document.createElement('div');
        this.thumbContainer = document.createElement('ul');
        this.thumbContainer.className = 'thumbs';

        // Insert them
        newElement.appendChild(this.thumbContainer);
        newElement.appendChild(this.contentContainer);
        element.parentNode.insertBefore(newElement, element);

        // Finally, remove the child element
        element.parentNode.removeChild(element);

        // Find all tabs, and build our tab items
        var nodes = <HTMLElement[]>_.filter(element.childNodes, n => n.nodeType === 1);
        for (var i = 0; i < nodes.length - 1; i += 2)
        {
            var dt = nodes[i],
                dd = nodes[i + 1];

            console.assert(dt.localName === 'dt' || dd.localName === 'dd');

            var thumb = document.createElement('li');
            thumb.dataset['index'] = i/2;
            thumb.addEventListener('click', e => this.setSelectedIndex((<HTMLLIElement>e.target).dataset['index']));

            // Move the thumb
            while (dt.childNodes.length !== 0)
                thumb.appendChild(dt.childNodes[0]);

            this.thumbContainer.appendChild(thumb);

            var content = document.createElement('div');

            // Move the tab page
            while (dd.childNodes.length !== 0)
                content.appendChild(dd.childNodes[0]);

            this.items.push({thumb: thumb, content: content});
        }

        this.setSelectedIndex(0);
    }

    public setSelectedIndex(index: number)
    {
        var tabItem = this.items[index];

        // Remove any existing content
        while (this.contentContainer.childNodes.length !== 0)
            this.contentContainer.removeChild(this.contentContainer.children[0]);

        // Set all thumbs as unselected
        for (var i = 0; i < this.items.length; i++)
            this.items[i].thumb.classList.remove('selected');

        this.items[index].thumb.classList.add('selected');
        this.contentContainer.appendChild(this.items[index].content);
    }
}

export = TabControl;