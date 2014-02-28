/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/jquery.d.ts" />
/// <reference path="../../libs/jointjs.d.ts" />

import constants = require('constants');
import data = require('data');
import control = require('control');
import Closeable = require('util/Closeable');
import state = require('state');
import Module = require('Module');

class OptionTreeModule extends Module
{
    private optionElementByName: {[name:string]:HTMLLIElement} = {};
    private optionList: HTMLUListElement;
    private graph: HTMLDivElement;

    constructor()
    {
        super('optiontree', 'option-tree');
    }

    public load(element: HTMLDivElement)
    {
        this.optionList = document.createElement('ul');
        this.optionList.className = 'options';
        element.appendChild(this.optionList);

        var usage = document.createElement('div');
        usage.className = 'control-container';
        control.buildSetting('options.announce-fsm-states', usage, this.closeables);
        element.appendChild(usage);

        this.closeables.add(new data.Subscription<state.OptionTree>(
            constants.protocols.optionTreeState,
            {
                onmessage: this.onData.bind(this)
            }
        ));

        var fsm = control.getFSM('playing');

        this.graph = document.createElement('div');
        this.graph.className = 'graph';
        element.appendChild(this.graph);

        // The model
        var graph = new joint.dia.Graph();

        // The view
        new joint.dia.Paper({
            el: this.graph,
            width: 640,
            height: 640,
            gridSize: 1,
            model: graph
        });

        // Create state elements
        _.each(fsm.states, state =>
        {
            var lines = state.id.split('\n');
            var maxLineLength = _.max(lines, l => l.length).length;

            // Compute width/height of the rectangle based on the number
            // of lines in the label and the letter size. 0.6 * letterSize is
            // an approximation of the monospaced font letter width.
            var letterSize = 12,
                minWidth = 90;
            var width = Math.max(minWidth, 0.6 * letterSize * maxLineLength);
            var height = 2 * lines.length * letterSize;

            var block = new joint.shapes.basic.Rect({
                id: state.id,
                size: { width: width, height: height },
                attrs: {
                    text: { text: state.id, 'font-size': letterSize, 'font-family': '"Ubuntu Mono", monospace' },
                    rect: {
                        width: width, height: height,
                        rx: 5, ry: 5,
                        stroke: '#792485'
                    }
                }
            });

            graph.addCell(block);
        });

        // Create transitions
        _.each(fsm.transitions, (transition: control.FSMTransition) =>
        {
            var link = new joint.dia.Link({
                source: { id: transition.from },
                target: { id: transition.to },
                attrs: { '.marker-target': { d: 'M 4 0 L 0 2 L 4 4 z' } },
                smooth: true
            });
            graph.addCell(link);
        });

        // Swallow mousedown events so that dragging graph elements
        // doesn't pick up the module instead.
        graph.on('batch:start', () => window.event.stopPropagation());

        // Perform layout
        (<any>joint).layout.DirectedGraph.layout(graph, { setLinkVertices: false });
    }

    public unload()
    {
        delete this.optionList;
        delete this.graph;
    }

    private onData(data: state.OptionTree)
    {
        _.each(this.optionElementByName, optionDiv => optionDiv.className = '');

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
    }
}

export = OptionTreeModule;
