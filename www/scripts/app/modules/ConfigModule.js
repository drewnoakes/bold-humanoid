/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'ControlClient',
        'DOMTemplate'
    ],
    function(ControlClient, DOMTemplate)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate('config-module-template');

        var ConfigModule = function()
        {
            this.container = document.createElement('div');

            this.title = 'config';
            this.id = 'config';
            this.panes = [
                {
                    title: 'main',
                    element: this.container,
                    supports: { fullScreen: false, advanced: false }
                }
            ];
        };

        ConfigModule.prototype.load = function()
        {
            var templateRoot = moduleTemplate.create();

            this.container.appendChild(templateRoot);

            this.textElement = this.container.querySelector('div.json-text');
            console.assert(this.textElement);

            this.subscription = ControlClient.onSettingChange(this.updateText.bind(this));

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
            this.textElement.textContent = ControlClient.getConfigText();
        };

        return ConfigModule;
    }
);
