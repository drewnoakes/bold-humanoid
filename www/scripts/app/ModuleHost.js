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

        ModuleHost.prototype.register = function(module)
        {
            if (this.loaded)
              throw 'Cannot register modules once the host is loaded.';

            this.modules.push(module);
        };

        var moduleTemplate = Handlebars.compile($('#module-template').html()),
            $moduleContainer = $('#module-container');

        ModuleHost.prototype.load = function()
        {
            if (this.modules.length === 0)
                throw 'No modules to load.';

            if (this.loaded)
              throw 'Already loaded.';

            this.loaded = true;
            this.addAllModules();
        };

        ModuleHost.prototype.addAllModules = function ()
        {
            _.each(this.modules, this.addModule.bind(this));
        };

        ModuleHost.prototype.addModule = function(module)
        {
            var moduleHtml = moduleTemplate(module),
                moduleElement = $('<div></div>').html(moduleHtml).children().get(0),
                $moduleElement = $(moduleElement);

            module.element = moduleElement;
            $moduleContainer.append(moduleElement);

            // Populate element properties
            module.paneContainer = $(moduleElement).find('.pane-container').get(0);

            if (module.load)
                module.load();

            var $links = $moduleElement.find('.pane-header-links');

            if (module.supports && module.supports.advanced)
            {
                var $advancedLink = $('<a></a>', {href:'#'}).text('advanced');
                $links.append($advancedLink);
                var isAdvanced = false;
                $advancedLink.click(function ()
                {
                    isAdvanced = !isAdvanced;
                    if (isAdvanced)
                    {
                        $moduleElement.addClass('advanced');
                        $advancedLink.text('basic');
                    }
                    else
                    {
                        $moduleElement.removeClass('advanced');
                        $advancedLink.text('advanced');
                    }
                    return false;
                })
            }

            var $closeLink = $('<a></a>', {href:'#'}).text('close');
            $closeLink.click(function(event) { event.preventDefault(); this.removeModule(module); return false; }.bind(this));
            $links.append($closeLink);

            // Load the first pane
            this.loadPane(module, module.panes[0]);
        };

        ModuleHost.prototype.removeModule = function(module)
        {
            if (!module.element)
                throw 'Has not been added';

            // Remove from the DOM
            $(module.element).remove();

            // Tell it to clean up
            if (module.unload)
                module.unload();
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