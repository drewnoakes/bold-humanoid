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
import Select = require('controls/Select');
import state = require('state');
import Trackable = require('util/Trackable');
import Module = require('Module');

class OptionTreeModule extends Module
{
    private optionElementByName: {[name:string]:HTMLLIElement} = {};
    private optionList: HTMLUListElement;
    private graph: HTMLDivElement;
    private selectedFSM: control.FSM;
    private paper: joint.dia.Paper;
    private linkByTransitionKey: {[name:string]:joint.dia.Link};
    private blockByStateId: {[name:string]:joint.shapes.basic.Rect};
    private highlightedLastCycle: SVGElement[] = [];

    constructor()
    {
        super('optiontree', 'option tree', {fullScreen: true});
    }

    public load(width: number)
    {
        var controls = document.createElement('div');
        controls.className = 'control-container flow';
        control.buildSettings('role-decider', controls, this.closeables);
        control.buildSetting('options.announce-fsm-states', controls, this.closeables);
        control.buildSetting('options.announce-fsm-transitions', controls, this.closeables);
        control.buildSettings('options.fsms', controls, this.closeables);
        this.element.appendChild(controls);

        this.closeables.add(new data.Subscription<state.OptionTree>(
            constants.protocols.optionTreeState,
            {
                onmessage: this.onOptionTreeState.bind(this)
            }
        ));

        this.graph = document.createElement('div');
        this.graph.className = 'graph';
        this.element.appendChild(this.graph);

        // The model
        var graph = new joint.dia.Graph();

        // Swallow mousedown events so that dragging graph elements
        // doesn't pick up the module instead.
        graph.on('batch:start', () => {
            !window.event || window.event.stopPropagation();
        });

        // The view
        this.paper = new joint.dia.Paper({
            el: this.graph,
            width: 640,
            height: 640,
            gridSize: 1,
            model: graph
        });

        // Control to select the visible FSM
        var visibleFsm = new Trackable<string>('play-mode');
        visibleFsm.track(fsmName => {
            var fsm = control.getFSM(fsmName);
            this.buildFsmGraph(fsm, graph);
        });
        var visibleFsmSelect = new Select(visibleFsm, _.map(control.getFsmDescriptions(), desc => { return {value:desc.name,text:desc.name}; }));
        controls.insertBefore(visibleFsmSelect.element, controls.firstChild);

        this.buildFsmGraph(control.getFSM('play-mode'), graph);

        this.optionList = document.createElement('ul');
        this.optionList.className = 'options';
        this.element.appendChild(this.optionList);
    }

    public unload()
    {
        delete this.optionList;
        delete this.graph;
    }

    private static createTransitionKey(transition: {id: string; to: string; from?: string;}): string
    {
        return (transition.from || '*') + '->' + transition.to + '[' + transition.id + ']';
    }

    private buildFsmGraph(fsm: control.FSM, graph: joint.dia.Graph)
    {
        this.selectedFSM = fsm;
        this.linkByTransitionKey = {};
        this.blockByStateId = {};

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
            var width = Math.round(Math.max(minWidth, 0.6 * letterSize * maxLineLength));
            var height = Math.round(1.6 * lines.length * letterSize);

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

            this.blockByStateId[state.id] = block;

            graph.addCell(block);

            this.paper.findViewByModel(block).el.addEventListener('click', (e: MouseEvent) =>
            {
                if (e.shiftKey)
                    control.getAction("options.fsms." + fsm.name + ".goto").activate({ state: state.id });
            });
        });

        // Create a wildcard 'from' node, if we have any wildcard transitions
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
            var link = new joint.dia.Link({
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
            });
            this.linkByTransitionKey[OptionTreeModule.createTransitionKey(transition)] = link;
            graph.addCell(link);
        });

        // Create wildcard transitions
        _.each(fsm.wildcardTransitions, (wildcardTransition: control.FSMWildcardTransition) =>
        {
            var link = new joint.dia.Link({
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
            });
            this.linkByTransitionKey[OptionTreeModule.createTransitionKey(wildcardTransition)] = link;
            graph.addCell(link);
        });

        // Perform layout
        var size = joint.layout.DirectedGraph.layout(graph, {
            setLinkVertices: false,
            rankSep: 45,
            edgeSep: 35,
            nodeSep: 25
        });

        this.paper.setDimensions(size.width + 2, size.height + 2);
    }

    private onOptionTreeState(optionTreeData: state.OptionTree)
    {
        // FSM GRAPH

        // Find selected FSM data
        var data = state.findOptionData(optionTreeData.path, this.selectedFSM.name);

        _.each(this.highlightedLastCycle, v => (<any>v).classList.remove("active"));
        this.highlightedLastCycle = [];

        if (data)
        {
            console.assert(data.type === 'FSM');

            var fsmData = <state.FSMOptionData>data.run;

            var stateName = fsmData.start;
            this.highlight(this.blockByStateId[ stateName]);

            // walk transitions, highlighting links and target states
            _.each(fsmData.transitions, d =>
            {
                var transitionKey = OptionTreeModule.createTransitionKey({
                    id: d.via,
                    to: d.to,
                    from: d.wildcard ? undefined : stateName
                });
                this.highlight(this.blockByStateId[d.to]);
                this.highlight(this.linkByTransitionKey[transitionKey]);
                stateName = d.to;
            });
        }

        // OPTION LIST

        _.each(this.optionElementByName, optionDiv => optionDiv.className = '');

        for (var i = 0; i < optionTreeData.ranoptions.length; i++)
        {
            var name = optionTreeData.ranoptions[i],
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

    private highlight(block: any)
    {
        var v: SVGElement = this.paper.findViewByModel(block).el;
        (<any>v).classList.add("active");
        this.highlightedLastCycle.push(v);
    }
}

export = OptionTreeModule;
