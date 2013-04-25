define(
    [
        'DataProxy',
        'Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

        var OptionTreeModule = function()
        {
            this.$container = $('<div></div>');
            this.options = {};

            this.title = 'option tree';
            this.id = 'optiontree';
            this.panes = [
                {
                    title: 'main',
                    element: this.$container
                }
            ];
        };
        
        OptionTreeModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(
                Protocols.optionTreeState,
                {
                    json: true,
                    onmessage: _.bind(this.onData, this)
                }
            );
        };

        OptionTreeModule.prototype.unload = function()
        {
            this.$container.empty();
            this.subscription.close();
        };

        OptionTreeModule.prototype.onData = function(data)
        {
            _.each(this.options, function (option)
            {
                option.ran = false;
            });

            for (var i = 0; i < data.ranoptions.length; i++)
            {
                var name = data.ranoptions[i];
                var option = this.options[name];
                if (option) {
                    option.ran = true;
                } else {
                    var view = $('<div>' + name + '</div>').addClass('option');
                    this.options[name] = {
                        view: view,
                        ran: true
                    };
                    this.$container.append(view);
                }
            }

            _.each(this.options, function (option)
            {
                option.view.toggleClass('ran', option.ran);
            });
        };

        return OptionTreeModule;
    }
)