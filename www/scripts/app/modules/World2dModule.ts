/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import control = require('control');
import HeadControls = require('controls/HeadControls');
import mapping = require('controls/mapping');
import Module = require('Module');
import Trackable = require('util/Trackable');
import geometry = require('util/geometry');

class World2dModule extends Module
{
    private map: mapping.Map;

    constructor()
    {
        super('world-2d', '2d world', {fullScreen: true});
    }

    public load()
    {
        var mapDiv = document.createElement('div');
        mapDiv.className = 'map-layer-container';

        var checkboxDiv = document.createElement('div');
        checkboxDiv.className = 'map-layer-checkboxes';

        var transform = new Trackable<geometry.Transform>();
        this.map = new mapping.Map(mapDiv, checkboxDiv, transform);

        var hoverInfo = document.createElement('div');
        hoverInfo.className = 'hover-info';

        this.map.hoverPoint.track(p => { hoverInfo.textContent = p ? p.x.toFixed(2) + ', ' + p.y.toFixed(2) : ''; });

        var localiserControlContainer = document.createElement('div');
        localiserControlContainer.className = 'localiser-controls';
        control.buildActions('localiser', localiserControlContainer);

        this.element.appendChild(mapDiv);
        this.element.appendChild(hoverInfo);
        this.element.appendChild(checkboxDiv);
        this.element.appendChild(new HeadControls().element);
        this.element.appendChild(localiserControlContainer);

        this.map.addLayer(new mapping.FieldLineLayer(transform));
        this.map.addLayer(new mapping.ParticleLayer(transform));
        this.map.addLayer(new mapping.ObservedLineLayer(transform));
        this.map.addLayer(new mapping.AgentPositionLayer(transform));
        this.map.addLayer(new mapping.VisibleFieldPolyLayer(transform));
        this.map.addLayer(new mapping.BallPositionLayer(transform));
        this.map.addLayer(new mapping.ObservedGoalLayer(transform));
        this.map.addLayer(new mapping.OcclusionAreaLayer(transform));
        this.map.addLayer(new mapping.TeamLayer(transform));
    }

    public unload()
    {
        this.map.unload();
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        // Make space for bottom controls if fullscreen
        if (isFullScreen)
            height -= 80;
        else
            height = 480;

        this.map.setPixelSize(width, height);
    }
}

export = World2dModule;
