/**
 * @author Drew Noakes https://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/three.d.ts" />
/// <reference path="../../libs/smoothie.d.ts" />

import Animator = require('Animator');
import constants = require('constants');
import control = require('control');
import threeUtil = require('util/three');
import data = require('data');
import dom = require('util/domdomdom');
import state = require('state');
import Legend = require('controls/Legend');
import Module = require('Module');

var chartHeight = 150;

var red = '#ED303C',
    grn = '#44C425',
    blu = '#00A8C6';

var pitchSeriesOptions = { strokeStyle: red, lineWidth: 1 };
var rollSeriesOptions  = { strokeStyle: grn, lineWidth: 1 };
var yawSeriesOptions   = { strokeStyle: blu, lineWidth: 1 };

var pi = String.fromCharCode(960);
var slash = ' ' + String.fromCharCode(8260) + ' ';
var piOn2 = pi + slash + '2';
var pi3On4 = '3' + pi + slash + '4';
var piOn4 = pi + slash + '4';

function toNiceAngleString(val: number) : string
{
    if (val > 0)
    {
        if (Math.abs(val - Math.PI) < 0.001)
            return pi;
        if (Math.abs(val - (3 * Math.PI / 4)) < 0.001)
            return pi3On4;
        if (Math.abs(val - (Math.PI / 2)) < 0.001)
            return piOn2;
        if (Math.abs(val - (Math.PI / 4)) < 0.001)
            return piOn4;
    }
    else
    {
        if (Math.abs(val + Math.PI) < 0.001)
            return '-' + pi;
        if (Math.abs(val + (3 * Math.PI / 4)) < 0.001)
            return '-' + pi3On4;
        if (Math.abs(val + (Math.PI / 2)) < 0.001)
            return '-' + piOn2;
        if (Math.abs(val + (Math.PI / 4)) < 0.001)
            return '-' + piOn4;
    }

    return val.toFixed(2);
}

var chartOptions: IChartOptions = {
    interpolation: 'step',
    grid: {
        strokeStyle: 'rgb(40, 40, 40)',
        fillStyle: 'rgb(0, 0, 0)',
        lineWidth: 1,
        millisPerLine: 250,
        verticalSections: 0,
        sharpLines: true,
        borderVisible: false
    },
    labels: {
        fillStyle: '#ffffff'
    },
    yRangeFunction: range =>
    {
        var max = Math.max(Math.abs(range.min), Math.abs(range.max));

        if (max < Math.PI/4)
            return {min:-Math.PI/4, max:Math.PI/4};
        if (max < Math.PI/2)
            return {min:-Math.PI/2, max:Math.PI/2};
        if (max < 3*Math.PI/4)
            return {min:-3*Math.PI/4, max:3*Math.PI/4};
        return {min:-Math.PI, max:Math.PI};
    },
    yMinFormatter: toNiceAngleString,
    yMaxFormatter: toNiceAngleString,
    horizontalLines: [
        {color: '#444444', lineWidth: 1, value: 3*Math.PI/4},
        {color: '#888888', lineWidth: 1, value: Math.PI/2},
        {color: '#444444', lineWidth: 1, value: Math.PI/4},
        {color: '#CCCCCC', lineWidth: 1, value: 0},
        {color: '#444444', lineWidth: 1, value: -Math.PI/4},
        {color: '#888888', lineWidth: 1, value: -Math.PI/2},
        {color: '#444444', lineWidth: 1, value: -3*Math.PI/4}
    ]
};

class OrientationModule extends Module
{
    private renderer: THREE.WebGLRenderer;
    private camera: THREE.PerspectiveCamera;
    private scene: THREE.Scene;
    private body: THREE.Object3D;
    private animator: Animator;

    private chart: SmoothieChart;
    private canvas: HTMLCanvasElement;
    private pitchSeries: TimeSeries;
    private rollSeries: TimeSeries;
    private yawSeries: TimeSeries;

    constructor()
    {
        super('orientation', 'orientation', { fullScreen: true });

        this.animator = new Animator(() => this.renderer.render(this.scene, this.camera));
    }

    public load(width: number)
    {
        this.initialiseScene();

        this.closeables.add(constants.isNightModeActive.track(
            isNightMode => this.renderer.setClearColor(isNightMode ? 0x211a20 : 0xcccccc, 1.0)));

        dom(this.element, this.renderer.domElement)
        this.element.appendChild(this.renderer.domElement);

        dom(this.element, new Legend([
            {name: "Pitch", colour: red},
            {name: "Roll",  colour: grn},
            {name: "Yaw",   colour: blu}
        ]).element);

        this.chart = new SmoothieChart(chartOptions);
        this.canvas = <HTMLCanvasElement>dom('canvas');
        this.canvas.height = chartHeight;
        this.canvas.width = width;
        this.element.appendChild(this.canvas);

        this.pitchSeries = new TimeSeries();
        this.rollSeries = new TimeSeries();
        this.yawSeries = new TimeSeries();
        this.chart.addTimeSeries(this.pitchSeries, pitchSeriesOptions);
        this.chart.addTimeSeries(this.rollSeries, rollSeriesOptions);
        this.chart.addTimeSeries(this.yawSeries, yawSeriesOptions);
        this.chart.streamTo(this.canvas, /*delayMs*/ 0);

        this.closeables.add(new data.Subscription<state.Orientation>(
            constants.protocols.orientationState,
            {
                onmessage: this.onOrientationState.bind(this)
            }
        ));

        var controlContainer = dom("div.controls");
        dom(this.element, controlContainer);
        control.buildActions("orientation-tracker", controlContainer);
        control.buildSettings("orientation-tracker", controlContainer, this.closeables);

        this.layout(width, 320, false);
        this.animator.start();
    }

    public unload()
    {
        this.animator.stop();

        this.chart.stop();

        delete this.body;
        delete this.scene;
        delete this.renderer;

        delete this.pitchSeries;
        delete this.rollSeries;
        delete this.yawSeries;
        delete this.chart;
        delete this.canvas;
    }

    private onOrientationState(data: state.Orientation)
    {
        // Quaternion values are provided as [x,y,z,w]
        this.body.quaternion.set(data.quaternion[0], data.quaternion[1], data.quaternion[2], data.quaternion[3]);
        this.animator.setRenderNeeded();

        var time = new Date().getTime();
        this.pitchSeries.append(time, data.pitch);
        this.rollSeries.append(time, data.roll);
        this.yawSeries.append(time, data.yaw);
    }

    private initialiseScene()
    {
        this.renderer = new THREE.WebGLRenderer({ antialias: true, devicePixelRatio: 1 });

//      this.camera = new THREE.OrthographicCamera(width / -2, width / 2, height / 2, height / -2, 1, 1000);
        this.camera = new THREE.PerspectiveCamera(55, 1, 0.001, 1);
        this.camera.position.y = -0.3;  // behind the agent, looking down +ve y
        this.camera.up.set(0, 0, 1);    // set the z-axis as 'up'
        this.camera.lookAt(new THREE.Vector3(0, 0, 0));

        this.scene = new THREE.Scene();

        //
        // Ambient light
        //
        this.scene.add(new THREE.AmbientLight(0x727876)); // standard fluorescent light

        //
        // Global directional light
        //
        var light = new THREE.DirectionalLight(0xffffff);
        light.position.set(-10, -2, 3).normalize();
        light.target.position.set(0, 0, 0);
        this.scene.add(light);

        //
        // Body
        //
        this.body = new THREE.Object3D();
        this.scene.add(this.body);

        var loader = new THREE.JSONLoader();
        var loadPart = (path: string, creaseAngle: number, offset?: {x?:number;y?:number;z?:number}) =>
        {
            loader.load(path, (geometry, materials) =>
            {
                _.each(materials, m =>
                {
                    m.opacity = 0.85;
                    m.transparent = true;
                    m.side = THREE.DoubleSide;
                    m.blending = THREE.NormalBlending;
                });

                geometry.computeFaceNormals();
                threeUtil.computeVertexNormals(geometry, creaseAngle);
                var mesh = new THREE.Mesh(geometry, new THREE.MeshFaceMaterial(materials));
                mesh.rotation.x = Math.PI / 2;
                mesh.rotation.y = Math.PI;
                if (offset && offset.x) mesh.position.x = offset.x;
                if (offset && offset.y) mesh.position.y = offset.y;
                if (offset && offset.z) mesh.position.z = offset.z;
                this.body.add(mesh);
                this.animator.setRenderNeeded();
            });
        };
        loadPart('models/darwin/darwin-body.json', 0.20);
        loadPart('models/darwin/darwin-neck.json', 0.20, {z: 0.051});
        loadPart('models/darwin/darwin-head.json', 1.00, {z: 0.051});
        loadPart('models/darwin/darwin-eye-led.json', 1.00, {z: 0.051});
        loadPart('models/darwin/darwin-forehead-led.json', 1.00, {z: 0.051});

        //
        // Axis helper
        // [R,G,B] === (x,y,z)
        //
        this.body.add(new THREE.AxisHelper(0.15));
        var referenceAxes = new THREE.AxisHelper(0.20);
        referenceAxes.material.transparent = true;
        referenceAxes.material.opacity = 0.3;
        this.scene.add(referenceAxes);
    }

    private layout(width: number, height: number, isFullScreen: boolean)
    {
        height = isFullScreen ? (height - chartHeight - 30) : width;

        this.renderer.setSize(width, height);
        this.camera.aspect = width / height;
        this.camera.updateProjectionMatrix();

        this.canvas.width = width;
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        this.layout(width, height, isFullScreen);
    }
}

export = OrientationModule;
