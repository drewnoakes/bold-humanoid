/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../libs/lodash.d.ts"/>

import DOMTemplate = require('DOMTemplate');
import Module = require('Module');
import util = require('util');

declare var jQuery;

var moduleTemplate = new DOMTemplate('module-template');

/* Modules are dynamic components, many instances of which make up the Round Table UI.
 *
 * ModuleHost provides common services to all modules, and controls their display on the page.
 */
class ModuleHost
{
    private loaded: boolean = false;
    private linkContainer: HTMLDivElement;
    private moduleContainer: HTMLDivElement;

    // Data per module...

    private moduleById: {[id:string]: Module} = {};
    private linkById: {[id:string]: HTMLAnchorElement} = {};
    private elementById: {[id:string]: HTMLDivElement} = {};

    constructor(linkContainerSelector: string)
    {
        this.linkContainer = <HTMLDivElement>document.querySelector(linkContainerSelector);
        this.moduleContainer = <HTMLDivElement>document.querySelector('#module-container');

        // Use jQuery sortable to allow modules to be reordered via drag/drop
        var sortableOptions = {
            update: this.updateHash.bind(this)
        };
        (<any>jQuery)(this.moduleContainer).sortable(sortableOptions); //.disableSelection();
    }

    public register(module: Module)
    {
        if (this.loaded)
          throw 'Cannot register modules once the ModuleHost is loaded.';

        this.moduleById[module.id] = module;

        var link = document.createElement('a');
        link.className = 'module-button';
        link.href = '#';
        link.textContent = module.title;
        link.addEventListener('click', event =>
        {
            event.preventDefault();
            if (this.elementById[module.id])
                this.removeModule(module);
            else
                this.addModule(module);
            return false;
        });
        this.linkById[module.id] = link;
        this.linkContainer.appendChild(link);
    }

    public load()
    {
        if (_.keys(this.moduleById).length === 0)
            throw 'No modules registered in ModuleHost.';

        if (this.loaded)
          throw 'ModuleHost already loaded.';

        this.loaded = true;

        // Load any modules found in the hash
        if (window.location.hash && window.location.hash.length > 1 && window.location.hash[0] === '#')
        {
            var moduleIds = window.location.hash.substr(1).split('|');
            _.each(moduleIds, moduleId =>
            {
                var module = this.moduleById[moduleId];
                if (module)
                    this.addModule(module);
            });
        }
    }

    private addModule(module: Module)
    {
        // Create a new element for the module
        var moduleElement = <HTMLDivElement>moduleTemplate.create(module);

        // Set the module link to show the module is 'added'
        var link = this.linkById[module.id];
        link.classList.add('added');

        this.elementById[module.id] = moduleElement;
        this.moduleContainer.appendChild(moduleElement);

        var headerLinks = moduleElement.querySelector('.module-header-links');
        var container = <HTMLDivElement>moduleElement.querySelector('.module-content');

        var closeLink = document.createElement('a');
        closeLink.href = '#';
        closeLink.textContent = 'close';
        closeLink.addEventListener('click', event =>
        {
            event.preventDefault();
            this.removeModule(module);
            return false;
        });
        headerLinks.appendChild(closeLink);

        util.clearChildren(container);

        module.load(container);

        module.onResized(container.clientWidth, container.clientHeight);

        this.updateHash();
    }

    private removeModule(module: Module)
    {
        var element = this.elementById[module.id];

        if (!element)
            throw 'Has not been added';

        // Remove from the DOM
        this.moduleContainer.removeChild(element);

        // Tell it to clean up
        module.unload();

        module.closeables.closeAll();

        delete this.elementById[module.id];

        this.linkById[module.id].classList.remove('added');

        this.updateHash();
    }

    private updateHash()
    {
        var hash = '';
        _.each(_.keys(this.elementById), moduleId =>
        {
            if (hash)
                hash += '|';
            hash += moduleId;
        });
        window.location.hash = hash;
    }
}

export = ModuleHost;
