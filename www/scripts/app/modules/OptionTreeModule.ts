/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'util/Closeable',
        'ControlBuilder',
        'DataProxy',
        'constants'
    ],
    function(Closeable, ControlBuilder, DataProxy, constants)
    {
        'use strict';

        var OptionTreeModule = function()
        {
            this.$container = $('<div></div>');
            this.optionElementByName = {};

            this.title = 'option tree';
            this.id = 'optiontree';
            this.element = this.$container.get(0);

            this.closables = new Closeable();
        };
        
        OptionTreeModule.prototype.load = function()
        {
            this.optionList = document.createElement('ul');
            this.optionList.className = 'options';
            this.$container.append(this.optionList);

            var usage = document.createElement('div');
            usage.className = 'control-container';
            this.closables.add(ControlBuilder.build('options.announce-fsm-states', usage));
            this.$container.append(usage);

            this.subscription = DataProxy.subscribe(
                constants.protocols.optionTreeState,
                {
                    json: true,
                    onmessage: this.onData.bind(this)
                }
            );
        };

        OptionTreeModule.prototype.unload = function()
        {
            this.$container.empty();
            this.subscription.close();
            this.closables.closeAll();

            delete this.optionList;
            delete this.subscription;
        };

        OptionTreeModule.prototype.onData = function(data)
        {
            _.each(this.optionElementByName, function(optionDiv)
            {
                optionDiv.className = '';
            });

            for (var i = 0; i < data.ranoptions.length; i++)
            {
                var name = data.ranoptions[i],
                    optionDiv = this.optionElementByName[name];

                if (optionDiv) {
                    optionDiv.classList.add('ran');
                } else {
                    optionDiv = document.createElement('li');
                    optionDiv.textContent = name;
                    optionDiv.className = 'ran';
                    this.optionElementByName[name] = optionDiv;
                    this.optionList.appendChild(optionDiv);
                }
            }
        };

        return OptionTreeModule;
    }
);