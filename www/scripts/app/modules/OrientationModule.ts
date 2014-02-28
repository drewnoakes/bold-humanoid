/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/three.d.ts" />

import constants = require('constants');
import ControlBuilder = require('ControlBuilder');
import threeUtil = require('util/three');
import data = require('data');
import state = require('state');
import Module = require('Module');

class OrientationModule extends Module
{
    private stopAnimation: boolean;
    private needsRender: boolean;

    private renderer: THREE.WebGLRenderer;
    private camera: THREE.PerspectiveCamera;
    private scene: THREE.Scene;
    private body: THREE.Object3D;

    constructor()
    {
        super('orientation', 'orientation');
    }

    public load(element: HTMLDivElement)
    {
        this.stopAnimation = false;
        this.needsRender = true;

        this.initialiseScene();

        element.appendChild(this.renderer.domElement);

        this.animate();

        this.closeables.add(new data.Subscription<state.Orientation>(
            constants.protocols.orientationState,
            {
                onmessage: this.onData.bind(this)
            }
        ));

        this.closeables.add(() => this.stopAnimation = true);

        ControlBuilder.actions("orientation-tracker", element);
        ControlBuilder.buildAll("orientation-tracker", element, this.closeables);
    }

    public unload()
    {
        delete this.body;
        delete this.scene;
        delete this.renderer;
    }

    private onData(data: state.Orientation)
    {
        // Data values are (w,x,y,z), but THREE.Quaternion needs them (x,y,z,w)
        this.body.quaternion.set(data.quaternion[1], data.quaternion[2], data.quaternion[3], data.quaternion[0]);
        this.needsRender = true;
    }

    private initialiseScene()
    {
        var width = 400,
            height = 400,
            aspect = width / height;

        this.renderer = new THREE.WebGLRenderer({ antialias: true, devicePixelRatio: 1 });
        this.renderer.setClearColor(0xcccccc, 1.0);
        this.renderer.setSize(width, height);

//      this.camera = new THREE.OrthographicCamera(width / -2, width / 2, height / 2, height / -2, 1, 1000);
        this.camera = new THREE.PerspectiveCamera(55, aspect, 0.001, 1);
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
                this.needsRender = true;
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

    private animate()
    {
        if (this.stopAnimation)
            return;

        window.requestAnimationFrame(this.animate.bind(this));

        if (!this.needsRender)
            return;

        this.renderer.render(this.scene, this.camera);
        this.needsRender = false;
    }
}

export = OrientationModule;
