define(
    [
        'DataProxy',
        'Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

        var GameStateModule = function()
        {
            this.container = $('<div>');
            this.options = {};

            this.title = 'game';
            this.id = 'game';
            this.panes = [
                {
                    title: 'main',
                    element: this.container
                }
            ];
        };
        
        GameStateModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(
                Protocols.gameState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        GameStateModule.prototype.unload = function()
        {
            this.subscription.close();
        };

        GameStateModule.prototype.onData = function(data)
        {
            // TODO populate a template with the data

            console.log(JSON.stringify(data));
        };

        return GameStateModule;
    }
)