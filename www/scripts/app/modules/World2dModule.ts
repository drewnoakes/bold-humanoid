/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import control = require('control');
import HeadControls = require('controls/HeadControls');
import mapping = require('controls/mapping');
import Module = require('Module');
import util = require('util');
import geometry = require('util/geometry');

class World2dModule extends Module
{
    private map: mapping.Map;
    private hoverInfo: HTMLDivElement;

    constructor()
    {
        super('world-2d', '2d world');
    }

    public load(element: HTMLDivElement)
    {
        var mapDiv = document.createElement('div');
        mapDiv.className = 'map-layer-container';

        var checkboxDiv = document.createElement('div');
        checkboxDiv.className = 'map-layer-checkboxes';

        var transform = new util.Trackable<geometry.Transform>();
        this.map = new mapping.Map(mapDiv, checkboxDiv, transform);

        this.hoverInfo = document.createElement('div');
        this.hoverInfo.className = 'hover-info';

        this.map.hoverPoint.track(p => { this.hoverInfo.textContent = p ? p.x.toFixed(2) + ', ' + p.y.toFixed(2) : ''; });

        var localiserControlContainer = document.createElement('div');
        localiserControlContainer.className = 'localiser-controls';
        control.buildActions('localiser', localiserControlContainer);

        element.appendChild(mapDiv);
        element.appendChild(checkboxDiv);
        element.appendChild(new HeadControls().element);
        element.appendChild(localiserControlContainer);
        element.appendChild(this.hoverInfo);

        this.map.addLayer(new mapping.FieldLineLayer(transform));
        this.map.addLayer(new mapping.ParticleLayer(transform));
        this.map.addLayer(new mapping.ObservedLineLayer(transform));
        this.map.addLayer(new mapping.AgentPositionLayer(transform));
        this.map.addLayer(new mapping.VisibleFieldPolyLayer(transform));
        this.map.addLayer(new mapping.BallPositionLayer(transform));
        this.map.addLayer(new mapping.ObservedGoalLayer(transform));
        this.map.addLayer(new mapping.OcclusionAreaLayer(transform));
    }

    public unload()
    {
        this.map.unload();
    }

    public onResized(width: number, height: number)
    {
        this.map.setPixelSize(width, height);
    }
}

export = World2dModule;
