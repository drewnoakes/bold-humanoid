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
        var ModuleHost = function (linkContainerSelector)
        {
            this.modules = [];
            this.loaded = false;
            this.$linkContainer = $(linkContainerSelector);
            this.$moduleContainer = $('#module-container');
            this.moduleById = {};

            var sortableOptions = {
                update: this.updateHash.bind(this)
            };
            this.$moduleContainer.sortable(sortableOptions); //.disableSelection();
        };

        ModuleHost.prototype.register = function(module)
        {
            if (this.loaded)
              throw 'Cannot register modules once the ModuleHost is loaded.';

            this.modules.push(module);

            this.moduleById[module.id] = module;

            module.__$button = $('<a></a>', {'class': 'module-button', href:'#'})
                .text(module.title)
                .click(function (event)
                {
                    event.preventDefault();
                    if (module.__element)
                        this.removeModule(module);
                    else
                        this.addModule(module);
                    return false;
                }.bind(this))
                .appendTo(this.$linkContainer);
        };

        var moduleTemplate = Handlebars.compile($('#module-template').html());

        ModuleHost.prototype.load = function()
        {
            if (this.modules.length === 0)
                throw 'No modules registered in ModuleHost.';

            if (this.loaded)
              throw 'ModuleHost already loaded.';

            this.loaded = true;

            // Load any modules found in the hash
            if (window.location.hash && window.location.hash.length > 1 && window.location.hash[0] === '#') {
                var moduleIds = window.location.hash.substr(1).split('|');
                _.each(moduleIds, function (moduleId)
                {
                    var module = this.moduleById[moduleId];
                    if (module)
                        this.addModule(module);
                }.bind(this));
            }
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

            module.__$button.addClass('added');
            module.__element = moduleElement;
            moduleElement.module = module;
            this.$moduleContainer.append(moduleElement);

            // Populate element properties
            module.__paneContainer = $(moduleElement).find('.pane-container').get(0);

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

            this.updateHash();
        };

        ModuleHost.prototype.removeModule = function(module)
        {
            if (!module.__element)
                throw 'Has not been added';

            module.__$button.removeClass('added');

            // Remove from the DOM
            $(module.__element).remove();

            // Tell it to clean up
            if (module.unload)
                module.unload();

            delete module.__element;

            this.updateHash();
        };

        ModuleHost.prototype.loadPane = function(module, pane)
        {
            $(module.__paneContainer).empty().append(pane.element);

            if (module.activePane && module.activePane.unload)
                module.activePane.unload();

            if (pane.load)
                pane.load();

            if (pane.onResized)
                pane.onResized(module.__paneContainer.clientWidth,  module.__paneContainer.clientHeight);

            module.activePane = pane;
        };

        ModuleHost.prototype.updateHash = function()
        {
            var hash = '';
            _.each(this.$moduleContainer.find('div.module'), function (moduleDiv)
            {
                if (hash)
                    hash += '|';
                hash += moduleDiv.module.id;
            });
            window.location.hash = hash;
        };

        return ModuleHost;
    }
);