/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/jquery.d.ts" />
/// <reference path="../../libs/jointjs.d.ts" />
/// <reference path="../../libs/joint.layout.DirectedGraph.d.ts" />

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

        var controls = document.createElement('div');
        controls.className = 'control-container';
        control.buildSetting('options.announce-fsm-states', controls, this.closeables);
        element.appendChild(controls);

        this.closeables.add(new data.Subscription<state.OptionTree>(
            constants.protocols.optionTreeState,
            {
                onmessage: this.onData.bind(this)
            }
        ));

        this.graph = document.createElement('div');
        this.graph.className = 'graph';
        element.appendChild(this.graph);

        // The model
        var graph = new joint.dia.Graph();

        // Swallow mousedown events so that dragging graph elements
        // doesn't pick up the module instead.
        graph.on('batch:start', () => window.event.stopPropagation());

        // The view
        new joint.dia.Paper({
            el: this.graph,
            width: 640,
            height: 640,
            gridSize: 1,
            model: graph
        });

        // Control to select the displayed FSM
        var select = document.createElement('select');
        _.each(control.getFsmDescriptions(), fsm =>
        {
            var option = document.createElement('option');
            option.text = fsm.name;
            option.value = fsm.name;
            select.appendChild(option);
        });
        select.addEventListener('change', () => this.buildFsmGraph(control.getFSM(select.options[select.selectedIndex].value), graph));
        controls.appendChild(select);

        this.buildFsmGraph(control.getFSM(select.options[0].value), graph);
    }

    public unload()
    {
        delete this.optionList;
        delete this.graph;
    }

    private buildFsmGraph(fsm: control.FSM, graph: joint.dia.Graph)
    {
        graph.clear();

        // Create state elements
        _.each(fsm.states, state =>
        {
            var lines = state.id.split('\n');
            var maxLineLength = _.max<string>(lines,l => l.length).length;

            // Compute width/height of the rectangle based on the number
            // of lines in the label and the letter size. 0.6 * letterSize is
            // an approximation of the monospaced font letter width.
            var letterSize = 12,
                minWidth = 90;
            var width = Math.max(minWidth, 0.6 * letterSize * maxLineLength);
            var height = 1.6 * lines.length * letterSize;

            var block = new joint.shapes.basic.Rect({
                id: state.id,
                size: { width: width, height: height },
                attrs: {
                    text: {
                        text: state.id,
                        'font-size': letterSize,
                        'font-family': '"Ubuntu Mono", monospace'
                    },
                    rect: {
                        width: width, height: height,
                        rx: 5, ry: 5,
                        stroke: '#792485'
                    }
                }
            });

            graph.addCell(block);
        });

        if (fsm.wildcardTransitions.length !== 0)
        {
            graph.addCell(new joint.shapes.basic.Circle({
                id: 'wildcard',
                size: { width: 20, height: 20 },
                attrs: {
                    circle: {
                        transform: 'translate(10, 10)',
                        r: 10,
                        fill: 'black'
                    }
                }
            }));
        }

        // Create transitions
        _.each(fsm.transitions, (transition: control.FSMTransition) =>
        {
            graph.addCell(new joint.dia.Link({
                source: { id: transition.from },
                target: { id: transition.to },
                attrs: {
                    '.marker-target': { d: 'M 4 0 L 0 2 L 4 4 z' },
                    '.connection': { opacity: 0.6 }
                },
                smooth: true,
                labels: [
                    {
                        position: 0.3,
                        attrs: {
                            rect: { fill: 'transparent' },
                            text: {
                                'font-size': 9,
                                text: transition.id
                            }
                        }
                    }
                ]
            }));
        });

        // Create wildcard transitions
        _.each(fsm.wildcardTransitions, (wildcardTransition: control.FSMWildcardTransition) =>
        {
            graph.addCell(new joint.dia.Link({
                source: { id: 'wildcard' },
                target: { id: wildcardTransition.to },
                attrs: {
                    '.marker-target': { d: 'M 4 0 L 0 2 L 4 4 z' },
                    '.connection': { 'stroke-dasharray': '2 3', opacity: 0.6 }
                },
                smooth: true,
                labels: [
                    {
                        position: 0.3,
                        attrs: {
                            rect: { fill: 'transparent' },
                            text: {
                                'font-size': 9,
                                text: wildcardTransition.id
                            }
                        }
                    }
                ]
            }));
        });

        // Perform layout
        joint.layout.DirectedGraph.layout(graph, {
            setLinkVertices: false,
            rankSep: 35,
            edgeSep: 35,
            nodeSep: 20
        });
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
