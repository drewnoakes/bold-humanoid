/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="jointjs.d.ts" />

interface Size
{
    width: number;
    height: number;
}

interface DiGraphLayoutOptions
{
    setLinkVertices?: boolean;
    debugLevel?: number;
    rankDir?: number;
    rankSep?: number;
    edgeSep?: number;
    nodeSep?: number;
}

declare module joint.layout.DirectedGraph
{
    function layout(graph: joint.dia.Graph, opt: DiGraphLayoutOptions): Size;
}
