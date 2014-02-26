/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlClient',
        'ControlBuilder',
        'DOMTemplate'
    ],
    function(ControlClient, ControlBuilder, DOMTemplate)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate('config-module-template');

        var ConfigModule = function()
        {
            this.container = document.createElement('div');

            this.title = 'config';
            this.id = 'config';
            this.element = this.container;
        };

        ConfigModule.prototype.load = function()
        {
            var templateRoot = moduleTemplate.create();

            this.container.appendChild(templateRoot);

            this.textElement = this.container.querySelector('div.json-text');
            console.assert(this.textElement);

            this.subscription = ControlClient.onSettingChange(this.updateText.bind(this));

            var header = templateRoot.querySelector('div.header');
            console.assert(header);
            ControlBuilder.actions('config', header);

            this.filter = document.createElement('input');
            this.filter.type = 'text';
            this.filter.placeholder = 'Type to filter...';
            this.filter.addEventListener('input', this.updateText.bind(this));
            header.appendChild(this.filter);

            this.updateText();
        };

        ConfigModule.prototype.unload = function()
        {
            while (this.container.hasChildNodes())
                this.container.removeChild(this.container.lastChild);

            delete this.textElement;

            if (this.subscription)
                this.subscription.close();
        };

        ConfigModule.prototype.updateText = function()
        {
            var matching = this.filter.value;
            this.textElement.textContent = ControlClient.getConfigText(matching);
        };

        return ConfigModule;
    }
);
