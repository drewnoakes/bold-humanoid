/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/GeometryUtil',
        'scripts/app/DataProxy',
        'scripts/app/Protocols',
        'scripts/app/Constants'
    ],
    function(GeometryUtil, DataProxy, Protocols, Constants)
    {
        'use strict';

        var ModelModule = function()
        {
            // camera variables
            this.useThirdPerson = true;
            this.cameraDistance = 0.4;
            this.cameraTheta = -Math.PI/4;
            this.cameraPhi = Math.PI/6;

            this.$element = $('<div></div>');
            this.element = this.$element.get(0);

            /////

            this.title = 'model';
            this.moduleClass = 'model';
            this.panes = [
                {
                    title: 'main',
                    element: this.element,
                    onResized: _.bind(this.onResized, this),
                    supports: { fullScreen: true }
                }
            ];
        };

        ModelModule.prototype.load = function()
        {
            var self = this;
            this.hinges = [];
            this.objectByName = {};
            this.isRenderQueued = false;

            this.initialiseScene();

            var firstPersonCheckbox = $('<input>', {type:'checkbox',id:'first-person-checkbox'});
            firstPersonCheckbox.change(function()
            {
                self.useThirdPerson = !firstPersonCheckbox.is(':checked');
                self.updateCameraPosition();
                self.render();
            });
            this.$element.append(firstPersonCheckbox);
            var firstPersonLabel = $('<label>', {'for':'first-person-checkbox', text:'First person view'});
            this.$element.append(firstPersonLabel);

            var root = this.buildBody(Constants.bodyStructure, function()
            {
                self.scene.add(root);
                self.render();
            });

            this.subscription = DataProxy.subscribe(Protocols.agentModel, { onmessage: _.bind(this.onMessage, this) });
        };

        ModelModule.prototype.unload = function()
        {
            this.subscription.cancel();
        };

        ModelModule.prototype.onResized = function(width, height)
        {
            this.renderer.setSize(width, height);
            this.camera.aspect = width / height;
            this.camera.updateProjectionMatrix();
        };

        ModelModule.prototype.onMessage = function(msg)
        {
            var floats = DataProxy.parseFloats(msg.data),
                angles = floats.slice(6);

            if (!angles || angles.length !== 20)
            {
                console.error("Expecting 20 angles");
                return;
            }

            var hasChange = false;
            for (var i = 0; i < 20; i++) {
                if (this.setHingeAngle(this.hinges[i + 1], angles[i])) {
                    hasChange = true;
                }
            }

            if (hasChange)
            {
                this.updateCameraPosition();
                this.render();
            }
        };

        ModelModule.prototype.setHingeAngle = function(hinge, angle)
        {
            if (!hinge.rotationAxis) {
                // No hinge is defined (eg: eyes)
                return false;
            }

            // Add any angular offset applied to this hinge (adjust the zero-position)
            if (hinge.rotationOrigin) {
                angle += hinge.rotationOrigin;
            }

            var rotation = hinge.rotationAxis.clone();
            rotation.multiplyScalar(angle);
            if (!hinge.rotation.equals(rotation)) {
                hinge.rotation = rotation;
                return true;
            }

            // No change was applied
            return false;
        };

        ModelModule.prototype.initialiseScene = function()
        {
            var self = this;
            this.scene = new THREE.Scene();
            this.scene.add(new THREE.AmbientLight(0x777777));

            this.pendingTextureCount = 3;

            var onTextureLoaded = function()
            {
                if (--self.pendingTextureCount === 0)
                    self.render();
            };

            //
            // Lighting
            //
            var light = new THREE.DirectionalLight(0xffffff);
            light.position.set(1, 2, 2);
            light.target.position.set(0, 0, 0);
            light.castShadow = true;
            light.shadowDarkness = 0.5;
//            light.shadowCameraVisible = true;
            light.shadowCameraNear = 2;
            light.shadowCameraFar = 6;
            var shadowBoxSize = 0.4;
            light.shadowCameraLeft = -shadowBoxSize;
            light.shadowCameraRight = shadowBoxSize;
            light.shadowCameraTop = shadowBoxSize;
            light.shadowCameraBottom = -shadowBoxSize;
            this.scene.add(light);

            //
            // Ground
            //
            var groundTexture = THREE.ImageUtils.loadTexture("images/felt.jpg", null, onTextureLoaded);
            groundTexture.anisotropy = 16;
            groundTexture.repeat.set(12, 12);
            groundTexture.wrapS = groundTexture.wrapT = THREE.RepeatWrapping;

            var groundMaterial = new THREE.MeshPhongMaterial({
                ambient: 0x555555,
                color: 0x008800,
                specular: 0x000000,
                shininess: 0,
                bumpMap: groundTexture,
                bumpScale: 0.05
            } );

            var groundSize = 3,
                groundPlaneY = -0.341,
                groundMesh = new THREE.Mesh(new THREE.PlaneGeometry(groundSize, groundSize), groundMaterial);
            groundMesh.position.y = groundPlaneY;
            groundMesh.rotation.x = -Math.PI/2;
            groundMesh.receiveShadow = true;
            this.scene.add(groundMesh);

            //
            // Ball
            //
            var ballMaterial = new THREE.MeshPhongMaterial({
                ambient: 0x666666,
                map: THREE.ImageUtils.loadTexture("images/ball-colour-map.png", null, onTextureLoaded),
                bumpMap: THREE.ImageUtils.loadTexture("images/ball-bump-map.jpg", null, onTextureLoaded),
                bumpScale: 0.0075
            } );

            var ballRadius = 0.037,
                ballSegments = 20,
                ballMesh = new THREE.Mesh(new THREE.SphereGeometry(ballRadius, ballSegments, ballSegments), ballMaterial);
            ballMesh.position.set(0.05, groundPlaneY + ballRadius, 0.1);
            ballMesh.castShadow = true;
            ballMesh.receiveShadow = false;
            this.scene.add(ballMesh);

            //
            // Render & Camera
            //
            var canvasWidth = 640, canvasHeight = 480;

            this.camera = new THREE.PerspectiveCamera( 75, canvasWidth / canvasHeight, 0.01, 100 );
//          this.camera = new THREE.OrthographicCamera(window.innerWidth / -2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / -2, 1, 1000);

            this.renderer = new THREE.WebGLRenderer({ antialias: true });
            this.renderer.setSize(canvasWidth, canvasHeight);
            this.renderer.shadowMapEnabled = true;
            this.renderer.shadowMapSoft = true;

            this.updateCameraPosition();
            this.render();

            this.element.appendChild(this.renderer.domElement);

            this.bindMouseInteraction(this.renderer.domElement);
        };

        ModelModule.prototype.buildBody = function(body, loadedCallback)
        {
            var geometriesToLoad = 0;

            var processNode = function(node, parentObject)
            {
                if (node.geometryPath) {
                    geometriesToLoad++;
                    var loader = new THREE.JSONLoader();
                    loader.load(node.geometryPath, function(geometry, materials)
                    {
                        geometry.computeFaceNormals();
//                        geometry.computeVertexNormals();
                        GeometryUtil.computeVertexNormals(geometry, node.creaseAngle || 0.2);
                        var object = new THREE.Mesh(geometry, new THREE.MeshFaceMaterial(materials));
                        object.castShadow = true;
                        object.receiveShadow  = false;
                        if (node.name) {
                            this.objectByName[node.name] = object;
                        }
                        parentObject.add(object);
                        geometriesToLoad--;
                        if (geometriesToLoad === 0) {
                            loadedCallback();
                        }
                    }.bind(this));
                }

                for (var i = 0; node.children && i < node.children.length; i++) {
                    // Create hinge objects to house the children
                    var childNode = node.children[i];
                    var childHinge = new THREE.Object3D();
                    if (childNode.offset) {
                        childHinge.position.x = childNode.offset.x || 0;
                        childHinge.position.y = childNode.offset.y || 0;
                    }
                    childHinge.rotationAxis = childNode.rotationAxis;
                    childHinge.rotationOrigin = childNode.rotationOrigin;
                    this.hinges[childNode.jointId] = childHinge;
                    this.setHingeAngle(childHinge, 0);
                    parentObject.add(childHinge);
                    processNode(childNode, childHinge);
                }
            }.bind(this);

            var root = new THREE.Object3D();
            processNode(body, root);

            return root;
        };

        ModelModule.prototype.bindMouseInteraction = function(container)
        {
            var onMouseDownPosition = new THREE.Vector2(),
                onMouseDownTheta = this.cameraTheta,
                onMouseDownPhi = this.cameraPhi,
                isMouseDown = false,
                self = this;

            container.addEventListener('mousewheel', function(event)
            {
                event.preventDefault();
                self.cameraDistance *= 1 - (event.wheelDeltaY/720);
                self.cameraDistance = Math.max(0.1, Math.min(5, self.cameraDistance));
                self.updateCameraPosition();
                self.render();
            });

            container.addEventListener('mousedown', function(event)
            {
                event.preventDefault();
                isMouseDown = true;
                onMouseDownTheta = self.cameraTheta;
                onMouseDownPhi = self.cameraPhi;
                onMouseDownPosition.x = event.clientX;
                onMouseDownPosition.y = event.clientY;
            });

            container.addEventListener('mousemove', function(event)
            {
                event.preventDefault();
                if (isMouseDown) {
                    self.cameraTheta = -((event.clientX - onMouseDownPosition.x) * 0.01) + onMouseDownTheta;
                    self.cameraPhi = ((event.clientY - onMouseDownPosition.y) * 0.01) + onMouseDownPhi;
                    self.cameraPhi = Math.min(Math.PI/2, Math.max(-Math.PI/2, self.cameraPhi));
                    self.updateCameraPosition();
                    self.render();
                }
            });

            container.addEventListener('mouseup', function(event)
            {
                event.preventDefault();
                isMouseDown = false;
                onMouseDownPosition.x = event.clientX - onMouseDownPosition.x;
                onMouseDownPosition.y = event.clientY - onMouseDownPosition.y;
            });
        };

        ModelModule.prototype.updateCameraPosition = function()
        {
            if (this.useThirdPerson) {
                // Third person -- position camera outside player
                this.camera.position.x = this.cameraDistance * Math.sin(this.cameraTheta) * Math.cos(this.cameraPhi);
                this.camera.position.y = this.cameraDistance * Math.sin(this.cameraPhi);
                this.camera.position.z = this.cameraDistance * Math.cos(this.cameraTheta) * Math.cos(this.cameraPhi);
                this.camera.lookAt(new THREE.Vector3(0, -0.1, 0));
            } else {
                // First person -- position camera in player's head
                var headMatrix = this.objectByName['head'].matrixWorld;
                this.camera.position = Constants.cameraOffsetInHead.clone().applyMatrix4(headMatrix);
                this.camera.lookAt(new THREE.Vector3(0, 0, 1).applyMatrix4(headMatrix));
            }
        };

        ModelModule.prototype.render = function()
        {
            // Only render once all textures are loaded
            if (this.pendingTextureCount !== 0)
                return;

            // Only render once per frame, regardless of how many times requested
            if (this.isRenderQueued)
                return;

            this.isRenderQueued = true;

            // Request render in the next frame
            var self = this;
            requestAnimationFrame(function()
            {
                self.isRenderQueued = false;
                self.renderer.render(self.scene, self.camera);
            });
        };

        return ModelModule;
    }
);
