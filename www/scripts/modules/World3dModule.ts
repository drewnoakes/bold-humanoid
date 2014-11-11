/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/lodash.d.ts" />
/// <reference path="../../libs/three.d.ts" />

import Animator = require('Animator');
import constants = require('constants');
import data = require('data');
import plotter = require('plotter');
import interaction = require('interaction');
import geometry = require('util/geometry');
import state = require('state');
import Module = require('Module');
import Checkbox = require('controls/Checkbox');
import Trackable = require('util/Trackable');
import BodyBuilder = require('util/BodyBuilder');

class World3dModule extends Module
{
    private cameraDistance: number = 0.55;
    private cameraTheta: number = -5.22;
    private cameraPhi: number = 0.34;
    private torsoHeight: number = 0.341;

    private useThirdPerson: Trackable<boolean>;
    private movePlayer: Trackable<boolean>;
    private drawObservedLines: Trackable<boolean>;
    private drawObservedGoals: Trackable<boolean>;
    private drawViewPoly: Trackable<boolean>;
    private showStaticObjects: Trackable<boolean>;
    private showCentreOfMass: Trackable<boolean>;

    private pendingTextureCount: number;

    private scene: THREE.Scene;
    private renderer: THREE.WebGLRenderer;
    private camera: THREE.PerspectiveCamera;
    private ballMesh: THREE.Object3D;
    private lineObject: THREE.Object3D;
    private bodyRoot: THREE.Object3D;
    private objectByName: {[name:string]:THREE.Mesh};
    private staticObjects: THREE.Object3D;
    private centreOfMassAxes: THREE.AxisHelper;

    private fieldLineMaterial: THREE.LineBasicMaterial;
    private visibleFieldPolyMaterial: THREE.LineBasicMaterial;
    private observedGoalMaterial: THREE.LineBasicMaterial;

    private animator: Animator;

    private hinges: BodyBuilder.Hinge[];

    constructor()
    {
        super('world-3d', '3d world', {fullScreen: true});

        this.animator = new Animator(this.render.bind(this));
    }

    public load(width: number)
    {
        this.hinges = [];
        this.objectByName = {};

        var controls = document.createElement('div');

        this.useThirdPerson = new Trackable<boolean>(true);
        this.useThirdPerson.onchange(() => { this.updateCameraPosition(); this.animator.setRenderNeeded(); });
        controls.appendChild(new Checkbox('Third person view', this.useThirdPerson).element);

        this.movePlayer = new Trackable<boolean>(true);
        this.movePlayer.onchange(() => { this.updateCameraPosition(); this.animator.setRenderNeeded(); });
        controls.appendChild(new Checkbox('Move player', this.movePlayer).element);

        this.drawObservedLines = new Trackable<boolean>(true);
        this.drawObservedLines.onchange(() => this.animator.setRenderNeeded());
        controls.appendChild(new Checkbox('Observed lines', this.drawObservedLines).element);

        this.drawObservedGoals = new Trackable<boolean>(true);
        this.drawObservedGoals.onchange(() => this.animator.setRenderNeeded());
        controls.appendChild(new Checkbox('Observed goals', this.drawObservedGoals).element);

        this.drawViewPoly = new Trackable<boolean>(false);
        this.drawViewPoly.onchange(() => this.animator.setRenderNeeded());
        controls.appendChild(new Checkbox('View poly', this.drawViewPoly).element);

        this.showStaticObjects = new Trackable<boolean>(true);
        this.showStaticObjects.onchange(value =>
        {
            this.staticObjects.traverse(child => child.visible = value);
            this.animator.setRenderNeeded();
        });
        controls.appendChild(new Checkbox('Show static objects', this.showStaticObjects).element);

        this.showCentreOfMass = new Trackable<boolean>(false);
        this.showCentreOfMass.onchange(() => this.animator.setRenderNeeded());
        controls.appendChild(new Checkbox('COM', this.showCentreOfMass).element);

        this.initialiseScene();

        this.element.appendChild(this.renderer.domElement);
        this.element.appendChild(controls);

        this.bodyRoot = BodyBuilder.buildBody(this.hinges, this.objectByName, () =>
        {
            this.centreOfMassAxes = new THREE.AxisHelper(0.2);
            this.bodyRoot.add(this.centreOfMassAxes);

            this.bodyRoot.position.z = this.torsoHeight;
            this.bodyRoot.rotation.z = -Math.PI/2;
            //this.bodyRoot.add(new THREE.AxisHelper(0.2)); // [R,G,B] === (x,y,z)
            this.scene.add(this.bodyRoot);
            this.animator.setRenderNeeded();
        });

        this.positionBodySpotlight(this.bodyRoot);

        this.closeables.add(new data.Subscription<state.Body>      (constants.protocols.bodyState,       { onmessage: this.onBodyState.bind(this) }));
        this.closeables.add(new data.Subscription<state.WorldFrame>(constants.protocols.worldFrameState, { onmessage: this.onWorldFrameState.bind(this) }));
        this.closeables.add(new data.Subscription<state.Hardware>  (constants.protocols.hardwareState,   { onmessage: this.onHardwareState.bind(this) }));

        this.animator.start();
    }

    public unload()
    {
        this.animator.stop();
    }

    public onResized(width: number, height: number, isFullScreen: boolean)
    {
        // Make space for bottom controls if fullscreen
        if (isFullScreen)
            height -= 20;
        else
            height = constants.cameraImageHeight;

        this.camera.aspect = width/height;
        this.camera.updateProjectionMatrix();
        this.animator.setRenderNeeded();

        this.renderer.setSize(width, height, true);
    }

    private onBodyState(data: state.Body)
    {
        console.assert(!!data.angles && data.angles.length === 20);

        if (this.showCentreOfMass.getValue())
        {
            this.centreOfMassAxes.position.set(data.com[0], data.com[1], data.com[2]);
            this.centreOfMassAxes.visible = true;
        }
        else
        {
            this.centreOfMassAxes.visible = false;
        }

        var hasChange = false;
        for (var i = 0; i < 20; i++) {
            if (this.hinges[i + 1].setAngle(data.angles[i])) {
                hasChange = true;
            }
        }

        if (hasChange)
        {
            this.updateAgentHeightFromGround();
            this.updateCameraPosition();
            this.animator.setRenderNeeded();
        }
    }

    private onWorldFrameState(data: state.WorldFrame)
    {
        if (this.movePlayer.getValue() && data.pos && data.pos instanceof Array && data.pos.length === 3) {
            this.bodyRoot.position.x = data.pos[0];
            this.bodyRoot.position.y = data.pos[1];
            this.bodyRoot.rotation.z = data.pos[2];
            this.positionBodySpotlight(this.bodyRoot);
        }

        if (data.ball && data.ball instanceof Array && data.ball.length === 3) {
            this.setBallPosition(data.ball[0], data.ball[1], data.ball[2]);
            this.ballMesh.visible = true;
        } else {
            this.ballMesh.visible = false;
        }

        // Clear any previous lines
        if (this.lineObject) {
            this.scene.remove(this.lineObject);
        }

        this.lineObject = new THREE.Object3D();
        this.scene.add(this.lineObject);

        if (this.drawObservedLines.getValue()) {
            if (data.lines && data.lines instanceof Array && data.lines.length !== 0) {
                _.each(data.lines, line =>
                {
                    var lineGeometry = new THREE.Geometry();
                    lineGeometry.vertices.push(new THREE.Vector3(line[0], line[1], /*line[2]*/0));
                    lineGeometry.vertices.push(new THREE.Vector3(line[3], line[4], /*line[5]*/0));
                    this.lineObject.add(new THREE.Line(lineGeometry, this.fieldLineMaterial));
                });
            }
        }

        if (this.drawViewPoly.getValue()) {
            var poly = data.visibleFieldPoly;
            if (poly && poly instanceof Array && poly.length !== 0) {
                var polyGeometry = new THREE.Geometry();
                _.each(poly, point =>
                {
                    polyGeometry.vertices.push(new THREE.Vector3(point[0], point[1], 0));
                });
                // close the loop
                polyGeometry.vertices.push(new THREE.Vector3(poly[0][0], poly[0][1], 0));
                this.lineObject.add(new THREE.Line(polyGeometry, this.visibleFieldPolyMaterial));
            }
        }

        if (this.drawObservedGoals.getValue()) {
            if (data.goals && data.goals instanceof Array && data.goals.length !== 0) {
                _.each(data.goals, goal =>
                {
                    var goalGeometry = new THREE.Geometry(),
                        radius = constants.goalPostDiameter/ 2,
                        midX = goal[0],
                        midY = goal[1],
                        stepCount = 18;
                    for (var i = 0; i <= stepCount; i++) {
                        var theta = (i / stepCount) * Math.PI * 2;
                        goalGeometry.vertices.push(new THREE.Vector3(
                            midX + Math.cos(theta) * radius,
                            midY + Math.sin(theta) * radius, 0));
                    }
                    this.lineObject.add(new THREE.Line(goalGeometry, this.observedGoalMaterial));
                });
            }
        }

        this.animator.setRenderNeeded();
    }

    private onHardwareState(data: state.Hardware)
    {
        // TODO only set if changed?

        if (data.eye && data.eye instanceof Array && data.eye.length === 3)
        {
            var eyeMaterial = <THREE.MeshPhongMaterial>(<THREE.MeshFaceMaterial>this.objectByName['eye-led'].material).materials[0];
            var eyeColor = new THREE.Color(0);

            eyeColor.r = data.eye[0];
            eyeColor.g = data.eye[1];
            eyeColor.b = data.eye[2];

            eyeMaterial.color = eyeColor;
            eyeMaterial.emissive = eyeColor;
        }

        if (data.forehead && data.forehead instanceof Array && data.forehead.length === 3)
        {
            var foreheadMaterial = <THREE.MeshPhongMaterial>(<THREE.MeshFaceMaterial>this.objectByName['forehead-led'].material).materials[0];
            var foreheadColor = new THREE.Color(0);

            foreheadColor.r = data.forehead[0];
            foreheadColor.g = data.forehead[1];
            foreheadColor.b = data.forehead[2];

            foreheadMaterial.color = foreheadColor;
            foreheadMaterial.emissive = foreheadColor;
        }
    }

    private updateAgentHeightFromGround()
    {
        if (!this.showStaticObjects.getValue())
            return;

        var leftFoot = this.objectByName['foot-left'];
        var rightFoot = this.objectByName['foot-right'];
        // Ensure we've loaded everything
        if (this.pendingTextureCount !== 0 || leftFoot == null || rightFoot == null)
            return;
        var leftZ = new THREE.Vector3().setFromMatrixPosition(leftFoot.matrixWorld).z;
        var rightZ = new THREE.Vector3().setFromMatrixPosition(rightFoot.matrixWorld).z;
        var error = Math.min(leftZ, rightZ) - constants.footHeight;
        if (Math.abs(error) > 0.01) {
            this.torsoHeight -= error;
            this.bodyRoot.position.z = this.torsoHeight;
            //this.updateCameraPosition();
            this.animator.setRenderNeeded();
        }
    }

    private initialiseScene()
    {
        this.scene = new THREE.Scene();
        this.scene.add(new THREE.AmbientLight(0x727876)); // standard fluorescent light

        this.staticObjects = new THREE.Object3D();
        this.scene.add(this.staticObjects);

        this.pendingTextureCount = 3;

        var onTextureLoaded = () =>
        {
            if (--this.pendingTextureCount === 0)
                this.animator.setRenderNeeded();
        };

        //
        // Global directional light
        //
        var light = new THREE.DirectionalLight(0x505550);
        light.position.set(-1, -1, 0).normalize();
        light.target.position.set(0, 0, 0);
        this.scene.add(light);

        //
        // Ground
        //
        var groundSizeX = constants.fieldX + 2*constants.outerMarginMinimum,
            groundSizeY = constants.fieldY + 2*constants.outerMarginMinimum,
            fieldLineCanvas = document.createElement('canvas'),
            fieldLineContext = fieldLineCanvas.getContext('2d'),
            scale = 200,
            plotOptions = {
                goalStrokeStyle: 'yellow',
                groundFillStyle: '#008800',
                lineStrokeStyle: '#ffffff'
            };
        fieldLineCanvas.width = groundSizeX * scale;
        fieldLineCanvas.height = groundSizeY * scale;
        new geometry.Transform().scale(scale, scale).translate(groundSizeX/2, groundSizeY/2).applyTo(fieldLineContext);
        plotter.drawField(fieldLineContext, plotOptions);
        plotter.drawFieldLines(fieldLineContext, plotOptions);

        var groundBumpMap = THREE.ImageUtils.loadTexture('resources/images/felt.jpg', null, texture =>
        {
            onTextureLoaded();
            // The image will be stretched to be the same size as the field lines
            // So we draw the felt texture repeatedly onto a large canvas, to avoid having large bumps
            var image = <HTMLImageElement>texture.image,
                bumpCanvas = document.createElement('canvas'),
                bumpContext = bumpCanvas.getContext('2d');
            bumpCanvas.width = groundSizeX * scale;
            bumpCanvas.height = groundSizeY * scale;
            for (var y = 0; y < bumpCanvas.height; y += image.height) {
                for (var x = 0; x < bumpCanvas.width; x += image.width) {
                    bumpContext.drawImage(image, x, y);
                }
            }
            texture.image = bumpCanvas;
            texture.needsUpdate = true;
        });
        groundBumpMap.anisotropy = 16;
        groundBumpMap.repeat.set(120, 120);
        groundBumpMap.wrapS = groundBumpMap.wrapT = THREE.RepeatWrapping;

        var fieldLineTexture = new THREE.Texture(<HTMLCanvasElement>fieldLineCanvas);
        fieldLineTexture.needsUpdate = true;
        fieldLineTexture.minFilter = THREE.LinearFilter;
        fieldLineTexture.magFilter = THREE.LinearFilter;
        var groundMaterial = new THREE.MeshPhongMaterial({
            ambient: 0x555555,
            map: fieldLineTexture,
            specular: 0x000000,
            shininess: 0,
            bumpMap: groundBumpMap,
            bumpScale: 0.025
        } );

        var groundMesh = new THREE.Mesh(new THREE.PlaneGeometry(groundSizeX, groundSizeY), groundMaterial);
        groundMesh.receiveShadow = true;
        this.staticObjects.add(groundMesh);

        //
        // Ball
        //
        var ballMaterial = new THREE.MeshPhongMaterial({
            ambient: 0x666666,
            map: THREE.ImageUtils.loadTexture("resources/images/ball-colour-map.png", null, onTextureLoaded),
            bumpMap: THREE.ImageUtils.loadTexture("resources/images/ball-bump-map.jpg", null, onTextureLoaded),
            bumpScale: 0.0075
        });

        var ballSegments = 20;
        this.ballMesh = new THREE.Mesh(new THREE.SphereGeometry(constants.ballRadius, ballSegments, ballSegments), ballMaterial);
        this.ballMesh.castShadow = true;
        this.ballMesh.receiveShadow = false;
        this.scene.add(this.ballMesh);
        this.setBallPosition(0.15, 0.05, 0);

        //
        // Goal posts
        //
        var goalMaterial = new THREE.MeshLambertMaterial({ color: 0xfcd116, ambient: 0x555555 }),
            goalRadius = constants.goalPostDiameter / 2,
            goalPostHeight = constants.goalZ + goalRadius,
            addGoalPost = (scaleX:number, scaleY:number) =>
            {
                var x = (constants.fieldX / 2 + goalRadius) * scaleX,
                    y = (constants.goalY / 2 + goalRadius) * scaleY,
                    cylinder = new THREE.Mesh(new THREE.CylinderGeometry(goalRadius, goalRadius, goalPostHeight, 36, 36, false), goalMaterial),
                    sphere = new THREE.Mesh(new THREE.SphereGeometry(goalRadius, 36, 36), goalMaterial);
                cylinder.rotation.x = Math.PI/2;
                cylinder.position.set(x, y, goalPostHeight/2);
                sphere.position.set(x, y, goalPostHeight);
                this.staticObjects.add(cylinder);
                this.staticObjects.add(sphere);
            },
            addGoalBar = (scaleX:number) =>
            {
                var x = (constants.fieldX / 2 + goalRadius) * scaleX,
                    barLength = constants.goalY + goalRadius * 2,
                    barHeight = constants.goalZ + goalRadius,
                    bar = new THREE.Mesh(new THREE.CylinderGeometry(goalRadius, goalRadius, barLength, 36, 36, false), goalMaterial);
                bar.position.set(x, 0, barHeight);
                this.staticObjects.add(bar);
            };

        addGoalPost(1,1);
        addGoalPost(-1,1);
        addGoalPost(1,-1);
        addGoalPost(-1,-1);
        addGoalBar(1);
        addGoalBar(-1);

        //
        // FIELD LINES
        //
        this.fieldLineMaterial = new THREE.LineBasicMaterial({ color: 0x0000ff });
        this.observedGoalMaterial = new THREE.LineBasicMaterial({ color: 0xffd700 });
        this.visibleFieldPolyMaterial = new THREE.LineBasicMaterial({ color: 0x004400, linewidth: 2 });

//            this.scene.add(new THREE.AxisHelper(1)); // [R,G,B] === (x,y,z)

        //
        // Render & Camera
        //
        var aspect = constants.cameraImageWidth / constants.cameraImageHeight,
            canvasHeight = constants.cameraImageHeight;

        this.camera = new THREE.PerspectiveCamera(constants.cameraFovVerticalDegrees, aspect, 0.01, 100);
//          this.camera = new THREE.OrthographicCamera(window.innerWidth / -2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / -2, 1, 1000);

        this.renderer = new THREE.WebGLRenderer({ antialias: true, devicePixelRatio: 1 });
        this.closeables.add(constants.isNightModeActive.track(
            isNightMode => this.renderer.setClearColor(isNightMode ? 0x211a20 : 0xcccccc, 1.0)));
        this.renderer.setSize(constants.cameraImageWidth, constants.cameraImageHeight, true);
        this.renderer.shadowMapEnabled = true;
        this.renderer.shadowMapType = THREE.PCFSoftShadowMap;

        this.updateCameraPosition();

        this.bindMouseInteraction(this.renderer.domElement);
    }

    private setBallPosition(x: number, y: number, z: number)
    {
        if (Math.abs(z) < 1e-3)
            z = constants.ballRadius;

        this.ballMesh.position.set(x, y, z);
    }

    private positionBodySpotlight(body: THREE.Object3D)
    {
        if (!this.showStaticObjects.getValue())
            return;

        var dist = 4,
            unit = Math.sqrt(dist*dist / 3),
            spotlight = (<any>body).spotlight;

        if (!spotlight)
        {
            var light = new THREE.DirectionalLight(0xffffff),
                shadowBoxSize = 0.4;
            light.castShadow = true;
            light.shadowDarkness = 0.3;
            //light.shadowCameraVisible = true; // useful for debugging
            light.shadowCameraNear = 2;
            light.shadowCameraFar = 6;
            light.shadowCameraLeft = -shadowBoxSize;
            light.shadowCameraRight = shadowBoxSize;
            light.shadowCameraTop = shadowBoxSize;
            light.shadowCameraBottom = -shadowBoxSize;
            this.scene.add(light);
            spotlight = (<any>body).spotlight = light;
        }

        spotlight.position.set(body.position.x + unit, body.position.y + unit, body.position.z + unit);
        spotlight.target.position.set(body.position.x, body.position.y, body.position.z); //.set(0, 0, 0);
    }

    private bindMouseInteraction(container: HTMLElement)
    {
        var onMouseDownTheta = this.cameraTheta,
            onMouseDownPhi = this.cameraPhi;

        container.addEventListener('mousewheel', e =>
        {
            if (!this.useThirdPerson.getValue())
                return;
            e.preventDefault();
            this.cameraDistance *= 1 - (e.wheelDelta/720);
            this.cameraDistance = Math.max(0.1, Math.min(5, this.cameraDistance));
            this.updateCameraPosition();
            this.animator.setRenderNeeded();
        });

        new interaction.Dragger(container, e => {
            if (e.isStart)
            {
                if (!this.useThirdPerson.getValue())
                    return;
                onMouseDownTheta = this.cameraTheta;
                onMouseDownPhi = this.cameraPhi;
            }
            else
            {
                if (!this.useThirdPerson.getValue())
                    return;
                this.cameraTheta = -(e.totalDeltaX * 0.01) + onMouseDownTheta;
                this.cameraPhi = (e.totalDeltaY * 0.01) + onMouseDownPhi;
                this.cameraPhi = Math.min(Math.PI / 2, Math.max(-Math.PI / 2, this.cameraPhi));
                this.updateCameraPosition();
                this.animator.setRenderNeeded();
            }
        });
    }

    private updateCameraPosition()
    {
        if (this.useThirdPerson.getValue()) {
            // Third person -- position camera outside player
            var torsoPosition = this.bodyRoot
                ? this.bodyRoot.position.clone().add(new THREE.Vector3(0,0,-0.05))
                : new THREE.Vector3(0, 0, 0);
            this.camera.position.x = this.cameraDistance * Math.sin(this.cameraTheta) * Math.cos(this.cameraPhi);
            this.camera.position.y = -this.cameraDistance * Math.cos(this.cameraTheta) * Math.cos(this.cameraPhi);
            this.camera.position.z = this.cameraDistance * Math.sin(this.cameraPhi);
            this.camera.position.add(torsoPosition);
            this.camera.up.set(0, 0, 1);
            this.camera.lookAt(torsoPosition);
        } else {
            // First person -- position camera in player's head
            var headMatrix = this.objectByName['head'].matrixWorld;
            var position = constants.cameraOffsetInHead.clone().applyMatrix4(headMatrix);
            this.camera.position.set(position.x, position.y, position.z);
            this.camera.lookAt(new THREE.Vector3(0, 0, 1).applyMatrix4(headMatrix));
        }
    }

    private render()
    {
        // Only render once all textures are loaded
        if (this.pendingTextureCount !== 0)
            return;

        this.renderer.render(this.scene, this.camera);
    }
}

export = World3dModule;
