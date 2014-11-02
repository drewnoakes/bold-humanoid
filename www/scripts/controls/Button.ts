/**
 * @author Drew Noakes https://drewnoakes.com
 */

class Button
{
    public element: HTMLButtonElement;

    constructor(label: string, callback: () => void)
    {
        this.element = <HTMLButtonElement>document.createElement('button');
        this.element.textContent = label;
        this.element.addEventListener('click', callback);
    }
}

export = Button;
