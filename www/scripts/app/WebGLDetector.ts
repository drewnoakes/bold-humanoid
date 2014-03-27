/**
 * @author alteredq / http://alteredqualia.com/
 * @author mr.doob / http://mrdoob.com/
 * @author Drew Noakes http://drewnoakes.com
 */

var global: any = window;

export var canvas = !!global.CanvasRenderingContext2D;
export var webgl: boolean;

try {
    webgl =  !!global.WebGLRenderingContext && !!document.createElement('canvas').getContext('experimental-webgl');
} catch (e) {
    webgl = false;
}

export var workers = !!global.Worker;
export var fileapi = !!(global.File && global.FileReader && global.FileList && global.Blob);

export function getWebGLErrorMessage()
{
    var domElement = document.createElement('div');

    domElement.style.fontFamily = 'monospace';
    domElement.style.fontSize = '13px';
    domElement.style.textAlign = 'center';
    domElement.style.background = '#eee';
    domElement.style.color = '#000';
    domElement.style.padding = '1em';
    domElement.style.width = '475px';
    domElement.style.margin = '5em auto 0';

    if (!this.webgl) {

        domElement.innerHTML = global.WebGLRenderingContext ? [
            'Your graphics card does not seem to support <a href="http://khronos.org/webgl/wiki/Getting_a_WebGL_Implementation">WebGL</a>.<br />',
            'Find out how to get it <a href="http://get.webgl.org/">here</a>.'
        ].join('\n') : [
            'Your browser does not seem to support <a href="http://khronos.org/webgl/wiki/Getting_a_WebGL_Implementation">WebGL</a>.<br/>',
            'Find out how to get it <a href="http://get.webgl.org/">here</a>.'
        ].join('\n');

    }

    return domElement;
}

export function addGetWebGLMessage(parameters)
{
    var parent, id, domElement;

    parameters = parameters || {};

    parent = parameters.parent !== undefined ? parameters.parent : document.body;
    id = parameters.id !== undefined ? parameters.id : 'oldie';

    domElement = getWebGLErrorMessage();
    domElement.id = id;

    parent.appendChild(domElement);
}

