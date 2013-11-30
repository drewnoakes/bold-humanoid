/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'Protocols',
        'DataProxy',
        'DOMTemplate'
    ],
    function(Protocols, DataProxy, DOMTemplate)
    {
        'use strict';

        var moduleTemplate = new DOMTemplate('state-dump-module-template');

        var StateDumpModule = function()
        {
            this.$container = $('<div></div>');

            this.title = 'state dump';
            this.id = 'state';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];
        };

        StateDumpModule.prototype.load = function()
        {
            var $templateRoot = $(moduleTemplate.create());

            this.$container.append($templateRoot);

            this.textElement = $templateRoot.find('div.json-text').get(0);

            var select = $templateRoot.find('select');

            select.append($('<option>').attr('value', '').text('(None)'));
            _.each(Protocols.allStates, function(stateName)
            {
                select.append($('<option>').attr('value', stateName).text(stateName));
            });

            var setState = function(state)
            {
                if (this.subscription) {
                    this.subscription.close();
                }

                $(this.textElement).text(state ? 'Waiting for an update...' : '');

                if (state) {
                    this.subscription = DataProxy.subscribe(
                        state,
                        {
                            json: true,
                            onmessage: _.bind(this.onData, this)
                        }
                    );
                }
            }.bind(this);

            select.change(function()
            {
                var state = select.get(0).value;
                setState(state);
            });
        };

        StateDumpModule.prototype.unload = function()
        {
            this.$container.empty();

            delete this.textElement;

            if (this.subscription)
                this.subscription.close();
        };

        StateDumpModule.prototype.onData = function(data)
        {
            $(this.textElement).text(JSON.stringify(data, undefined, 2));
        };

        return StateDumpModule;
    }
);
