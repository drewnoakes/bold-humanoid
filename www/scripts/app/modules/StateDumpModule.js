/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/Protocols',
        'scripts/app/DataProxy'
    ],
    function(Protocols, DataProxy)
    {
        'use strict';

        var StateDumpModule = function()
        {
            var moduleHtml = Handlebars.compile($('#state-module-template').html()),
                container = $('<div></div>').html(moduleHtml).children(),
                select = container.find('select');

            this.textElement = container.find('div.json-text').get(0);

            /////

            this.title = 'state dump';
            this.id = 'state';
            this.panes = [
                {
                    title: 'main',
                    element: container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];

            var stateNames = Protocols.allStates;

            select.append($("<option>").attr('value', '').text('(None)'));
            _.each(stateNames, function(stateName)
            {
                select.append($("<option>").attr('value', stateName).text(stateName));
            });

            var setState = function(state)
            {
                if (this.subscription) {
                    this.subscription.close();
                }

                this.textElement.innerText = state ? 'Waiting for an update...' : '';

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

        StateDumpModule.prototype.load = function()
        {};

        StateDumpModule.prototype.unload = function()
        {
            if (this.subscription)
                this.subscription.close();
        };

        StateDumpModule.prototype.onData = function(data)
        {
            this.textElement.innerText = JSON.stringify(data, undefined, 2);
        };

        return StateDumpModule;
    }
);
