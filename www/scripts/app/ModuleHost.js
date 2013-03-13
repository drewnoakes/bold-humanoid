/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        /* Modules are dynamic components, many instances of which make up the Round Table UI.
         *
         * ModuleHost provides common services to all modules, and controls their display on the page.
         */
        var ModuleHost = function ()
        {
            this.modules = [];
            this.loaded = false;
        };

        ModuleHost.prototype.add = function(module)
        {
            if (this.loaded)
              throw 'Cannot add modules once the host is loaded.';

            this.modules.push(module);
        };

        ModuleHost.prototype.load = function()
        {
            if (this.modules.length === 0)
                throw 'No modules to load.';

            this.loaded = true;

            var moduleTemplate = Handlebars.compile($('#module-template').html()),
                $moduleContainer = $('#module-container'),
                self = this;

            _.each(this.modules, function(module)
            {
                var moduleHtml = moduleTemplate(module),
                    moduleElement = $('<div></div>').html(moduleHtml).children().get(0);

                $moduleContainer.append(moduleElement);

                // Populate element properties
                module.paneContainer = $(moduleElement).find('.pane-container').get(0);

                if (module.load)
                    module.load();

                // Load the first pane
                self.loadPane(module, module.panes[0]);
            });
        };

        ModuleHost.prototype.loadPane = function(module, pane)
        {
            $(module.paneContainer).empty().append(pane.element);

            if (module.activePane && module.activePane.unload)
                module.activePane.unload();

            if (pane.load)
                pane.load();

            if (pane.onResized)
                pane.onResized(module.paneContainer.clientWidth,  module.paneContainer.clientHeight);

            module.activePane = pane;
        };

        return ModuleHost;
    }
);