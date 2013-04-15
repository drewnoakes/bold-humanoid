/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/DataProxy'
    ],
    function(DataProxy)
    {
        'use strict';

        var StateModule = function()
        {
            var moduleHtml = Handlebars.compile($('#state-module-template').html()),
                container = $('<div></div>').html(moduleHtml).children(),
                select = container.find('select');

            this.textElement = container.find('div.json-text').get(0);

            /////

            this.title = 'state';
            this.moduleClass = 'state';
            this.panes = [
                {
                    title: 'main',
                    element: container.get(0),
                    supports: { fullScreen: true, advanced: false }
                }
            ];

            // TODO populate this from the server somehow
            var stateNames = ['AgentFrame','CameraFrame','WorldFrame','Body','Game','Hardware'];

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

                this.textElement.innerText = '';

                if (state) {
                    this.subscription = DataProxy.subscribe(state, { onmessage: _.bind(this.onmessage, this) });
                }
            }.bind(this);

            select.change(function()
            {
                var state = select.get(0).selectedOptions[0].value;
                setState(state);
            });
        };

        StateModule.prototype.load = function()
        {};

        StateModule.prototype.unload = function()
        {
            this.subscription.close();
        };

        StateModule.prototype.onmessage = function(msg)
        {
            var text = JSON.stringify(JSON.parse(msg.data), undefined, 2);
            this.textElement.innerText = text;
        };

        return StateModule;
    }
);
