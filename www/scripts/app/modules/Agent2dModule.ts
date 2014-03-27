/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />

import control = require('control');
import HeadControls = require('controls/HeadControls');
import geometry = require('util/geometry');
import mapping = require('controls/mapping');
import Module = require('Module');
import Trackable = require('util/Trackable');

class Agent2dModule extends Module
{
    private map: mapping.Map;

    constructor()
    {
        super('agent-2d', '2d agent');
    }

    public load(element: HTMLDivElement)
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

        element.appendChild(mapDiv);
        element.appendChild(hoverInfo);
        element.appendChild(checkboxDiv);
        element.appendChild(new HeadControls().element);
        element.appendChild(localiserControlContainer);

        this.map.addLayer(new mapping.AgentReferenceLayer(transform));
        this.map.addLayer(new mapping.AgentObservedLineLayer(transform));
        this.map.addLayer(new mapping.AgentObservedLineJunctionLayer(transform));
        this.map.addLayer(new mapping.AgentVisibleFieldPolyLayer(transform));
        this.map.addLayer(new mapping.AgentBallPositionLayer(transform));
        this.map.addLayer(new mapping.AgentObservedGoalLayer(transform));
        this.map.addLayer(new mapping.AgentOcclusionAreaLayer(transform));
    }

    public onResized(width, height)
    {
        var scale = this.map.transform.getValue().getScale();
//        var scale = Math.min(width / 12, height / 12);
        this.map.transform.setValue(
            new geometry.Transform()
            .translate(width / 2, height / 2)
            .scale(scale, -scale));

        this.map.setPixelSize(width, height);
    }
}

export = Agent2dModule;
