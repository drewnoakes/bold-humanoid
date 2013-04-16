define(
    [
        'scripts/app/DataProxy',
        'scripts/app/Protocols'
    ],
    function(DataProxy, Protocols)
    {
        'use strict';

        var OptionTreeModule = function()
        {
            this.container = $('<div>');
            this.options = {};

            this.title = 'optiontree';
            this.moduleClass = 'optiontree';
            this.panes = [
                {
                    title: 'main',
                    element: this.container
                }
            ];
        };
        
        OptionTreeModule.prototype.load = function()
        {
            this.subscription = DataProxy.subscribe(Protocols.optionTreeState, { onmessage: _.bind(this.onmessage, this) });
        };

        OptionTreeModule.prototype.unload = function()
        {
            this.subscription.close();
        };

        OptionTreeModule.prototype.onmessage = function(msg)
        {
            for (var name in this.options)
                this.options[name].ran = false;

            var data = JSON.parse(msg.data);
            for (var i = 0; i < data.ranoptions.length; ++i)
            {
                var name = data.ranoptions[i];
                var option = this.options[name];
                if (!option)
                {
                    option = this.options[name] = {
                        view: $('<div>'+name+'</div>').addClass('option'),
                        ran: true
                    };
                    this.container.append(option.view);
                }
                option.ran = true;
            }

            for (var name in this.options) {
                var option = this.options[name];
                if (option.ran) {
                    option.view.addClass('ran');
                } else {
                    option.view.removeClass('ran');
                }
            }
        };

        return OptionTreeModule;
    }
)