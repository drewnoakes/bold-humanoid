/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'GeometryUtil',
        'DataProxy',
        'Protocols',
        'FieldLinePlotter',
        'Constants',
        'util/Dragger'
    ],
    function(GeometryUtil, DataProxy, Protocols, FieldLinePlotter, Constants, Dragger)
    {
        'use strict';

        var World3dModule = function()
        {
            // camera variables
            this.useThirdPerson = true;
            this.cameraDistance = 0.55;
            this.cameraTheta = -5.22;
            this.cameraPhi = 0.34;
            this.torsoHeight = 0.341;

            this.$element = $('<div></div>');
            this.element = this.$element.get(0);

            /////

            this.title = '3d world';
            this.id = 'world-3d';
            this.panes = [
                {
                    title: 'main',
                    element: this.element,
                    supports: { fullScreen: true }
                }
            ];
        };

        World3dModule.prototype.load = function()
        {
            this.hinges = [];
            this.objectByName = {};
            this.isRenderQueued = false;

            this.initialiseScene();

            var addCheckbox = function(id, text, checked, onchange)
            {
                var checkbox = $('<input>', {type:'checkbox',id:id}).change(function() { onchange(checkbox.is(':checked')); }.bind(this)).attr('checked',!!checked);

                this.$element
                    .append(checkbox)
                    .append($('<label>', {'for':id, text:text}));

                onchange(!!checked);
            }.bind(this);

            addCheckbox('first-person-checkbox', 'First person view', false, function(isChecked)
            {
                this.useThirdPerson = !isChecked;
                this.updateCameraPosition();
                this.render();
            }.bind(this));

            addCheckbox('move-player-checkbox', 'Move player', true, function(isChecked)
            {
                this.movePlayer = isChecked;
                this.updateCameraPosition();
                this.render();
            }.bind(this));

            addCheckbox('draw-lines-checkbox', 'Draw observed lines', true, function(isChecked)
            {
                this.drawObservedLines = isChecked;
                this.render();
            }.bind(this));

            addCheckbox('draw-view-poly-checkbox', 'Draw view poly', false, function(isChecked)
            {
                this.drawViewPoly = isChecked;
                this.render();
            }.bind(this));

            this.bodyRoot = this.buildBody(Constants.bodyStructure, function()
            {
                this.bodyRoot.position.z = this.torsoHeight;
                this.bodyRoot.rotation.z = -Math.PI/2;
                //this.bodyRoot.add(new THREE.AxisHelper(0.2)); // [R,G,B] === (x,y,z)
                this.scene.add(this.bodyRoot);
                this.render();
            }.bind(this));

            this.positionBodySpotlight(this.bodyRoot);

            this.bodyStateSubscription  = DataProxy.subscribe(Protocols.bodyState,       { json: true, onmessage: _.bind(this.onBodyStateData, this) });
            this.worldFrameSubscription = DataProxy.subscribe(Protocols.worldFrameState, { json: true, onmessage: _.bind(this.onWorldFrameData, this) });
            this.hardwareSubscription   = DataProxy.subscribe(Protocols.hardwareState,   { json: true, onmessage: _.bind(this.onHardwareData, this) });
        };

        World3dModule.prototype.unload = function()
        {
            this.bodyStateSubscription.close();
            this.worldFrameSubscription.close();
            this.hardwareSubscription.close();
        };

        World3dModule.prototype.onBodyStateData = function(data)
        {
            if (!data.angles || data.angles.length !== 20)
            {
                console.error("Expecting 20 angles");
                return;
            }

            var hasChange = false;
            for (var i = 0; i < 20; i++) {
                if (this.setHingeAngle(this.hinges[i + 1], data.angles[i])) {
                    hasChange = true;
                }
            }

            if (hasChange)
            {
                this.updateAgentHeightFromGround();
                this.updateCameraPosition();
                this.render();
            }
        };

        World3dModule.prototype.onWorldFrameData = function(data)
        {
            if (this.movePlayer && data.pos && data.pos instanceof Array && data.pos.length === 3) {
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

            if (this.drawObservedLines) {
                if (data.lines && data.lines instanceof Array && data.lines.length !== 0) {
                    _.each(data.lines, function (line)
                    {
                        var lineGeometry = new THREE.Geometry();
                        lineGeometry.vertices.push(new THREE.Vector3(line[0], line[1], /*line[2]*/0));
                        lineGeometry.vertices.push(new THREE.Vector3(line[3], line[4], /*line[5]*/0));
                        this.lineObject.add(new THREE.Line(lineGeometry, this.fieldLineMaterial));
                    }.bind(this));
                }
            }

            if (this.drawViewPoly) {
                var poly = data['visible-field-poly'];
                if (poly && poly instanceof Array && poly.length !== 0) {
                    var polyGeometry = new THREE.Geometry();
                    _.each(poly, function (point)
                    {
                        polyGeometry.vertices.push(new THREE.Vector3(point[0], point[1], 0));
                    }.bind(this));
                    // close the loop
                    polyGeometry.vertices.push(new THREE.Vector3(poly[0][0], poly[0][1], 0));
                    this.lineObject.add(new THREE.Line(polyGeometry, this.visibleFieldPolyMaterial));
                }
            }

            this.render();
        };

        World3dModule.prototype.onHardwareData = function(data)
        {
            // TODO only set if changed?

            if (data.eye && data.eye instanceof Array && data.eye.length === 3)
            {
                var eyeMaterial = this.objectByName['eye-led'].material.materials[0];
                var eyeColor = new THREE.Color(0);

                eyeColor.r = data.eye[0];
                eyeColor.g = data.eye[1];
                eyeColor.b = data.eye[2];

                eyeMaterial.color = eyeColor;
                eyeMaterial.emissive = eyeColor;
            }

            if (data.forehead && data.forehead instanceof Array && data.forehead.length === 3)
            {
                var foreheadMaterial = this.objectByName['forehead-led'].material.materials[0];
                var foreheadColor = new THREE.Color(0);

                foreheadColor.r = data.forehead[0];
                foreheadColor.g = data.forehead[1];
                foreheadColor.b = data.forehead[2];

                foreheadMaterial.color = foreheadColor;
                foreheadMaterial.emissive = foreheadColor;
            }
        };

        World3dModule.prototype.updateAgentHeightFromGround = function()
        {
            var leftZ = new THREE.Vector3().getPositionFromMatrix(this.objectByName['foot-left'].matrixWorld).z;
            var rightZ = new THREE.Vector3().getPositionFromMatrix(this.objectByName['foot-right'].matrixWorld).z;
            var error = Math.min(leftZ, rightZ) - Constants.footHeight;
            if (Math.abs(error) > 0.01) {
                this.torsoHeight -= error;
                this.bodyRoot.position.z = this.torsoHeight;
                //this.updateCameraPosition();
                this.render();
            }
        };

        World3dModule.prototype.setHingeAngle = function(hinge, angle)
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

        World3dModule.prototype.initialiseScene = function()
        {
            this.scene = new THREE.Scene();
            this.scene.add(new THREE.AmbientLight(0x727876)); // standard fluorescent light

            this.pendingTextureCount = 3;

            var onTextureLoaded = function()
            {
                if (--this.pendingTextureCount === 0)
                    this.render();
            }.bind(this);

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
            var groundSizeX = Constants.fieldX + 2*Constants.outerMarginMinimum,
                groundSizeY = Constants.fieldY + 2*Constants.outerMarginMinimum,
                fieldLineCanvas = document.createElement('canvas'),
                fieldLineContext = fieldLineCanvas.getContext('2d'),
                scale = 700,
                plotOptions = {
                    scale: scale,
                    goalStrokeStyle: 'yellow',
                    groundFillStyle: '#008800',
                    lineStrokeStyle: '#ffffff',
                    fieldCenter: { x: scale*groundSizeX/2, y: scale*groundSizeY/2 }
                };
            fieldLineCanvas.width = groundSizeX * scale;
            fieldLineCanvas.height = groundSizeY * scale;
            FieldLinePlotter.start(fieldLineContext, plotOptions);
            FieldLinePlotter.drawFieldLines(fieldLineContext, plotOptions);
            FieldLinePlotter.end(fieldLineContext);

            var groundBumpMap = THREE.ImageUtils.loadTexture('images/felt.jpg', null, function(texture)
            {
                onTextureLoaded();
                // The image will be stretched to be the same size as the field lines
                // So we draw the felt texture repeatedly onto a large canvas, to avoid having large bumps
                var image = texture.image,
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

            var fieldLineTexture = new THREE.Texture(fieldLineCanvas);
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
            this.scene.add(groundMesh);

            //
            // Ball
            //
            var ballMaterial = new THREE.MeshPhongMaterial({
                ambient: 0x666666,
                map: THREE.ImageUtils.loadTexture("images/ball-colour-map.png", null, onTextureLoaded),
                bumpMap: THREE.ImageUtils.loadTexture("images/ball-bump-map.jpg", null, onTextureLoaded),
                bumpScale: 0.0075
            });

            var ballSegments = 20;
            this.ballMesh = new THREE.Mesh(new THREE.SphereGeometry(Constants.ballRadius, ballSegments, ballSegments), ballMaterial);
            this.ballMesh.castShadow = true;
            this.ballMesh.receiveShadow = false;
            this.scene.add(this.ballMesh);
            this.setBallPosition(0.15, 0.05);

            //
            // Goal posts
            //
            var goalMaterial = new THREE.MeshLambertMaterial({ color: 0xfcd116, ambient: 0x555555 }),
                goalRadius = Constants.goalPostDiameter / 2,
                goalPostHeight = Constants.goalZ + goalRadius,
                addGoalPost = function(scaleX, scaleY)
                {
                    var x = (Constants.fieldX / 2 + goalRadius) * scaleX,
                        y = (Constants.goalY / 2 + goalRadius) * scaleY,
                        cylinder = new THREE.Mesh(new THREE.CylinderGeometry(goalRadius, goalRadius, goalPostHeight, 36, 36, false), goalMaterial),
                        sphere = new THREE.Mesh(new THREE.SphereGeometry(goalRadius, 36, 36), goalMaterial);
                    cylinder.rotation.x = Math.PI/2;
                    cylinder.position.set(x, y, goalPostHeight/2);
                    sphere.position.set(x, y, goalPostHeight);
                    this.scene.add(cylinder);
                    this.scene.add(sphere);
                }.bind(this),
                addGoalBar = function(scaleX)
                {
                    var x = (Constants.fieldX / 2 + goalRadius) * scaleX,
                        barLength = Constants.goalY + goalRadius * 2,
                        barHeight = Constants.goalZ + goalRadius,
                        bar = new THREE.Mesh(new THREE.CylinderGeometry(goalRadius, goalRadius, barLength, 36, 36, false), goalMaterial);
                    bar.position.set(x, 0, barHeight);
                    this.scene.add(bar);
                }.bind(this);

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
            this.visibleFieldPolyMaterial = new THREE.LineBasicMaterial({ color: 0x004400, linewidth: 2 });

//            this.scene.add(new THREE.AxisHelper(1)); // [R,G,B] === (x,y,z)

            //
            // Render & Camera
            //
            var aspect = Constants.cameraWidth / Constants.cameraHeight,
                canvasHeight = Constants.cameraHeight;

            this.camera = new THREE.PerspectiveCamera(Constants.cameraFovVerticalDegrees, aspect, 0.01, 100);
//          this.camera = new THREE.OrthographicCamera(window.innerWidth / -2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / -2, 1, 1000);

            this.renderer = new THREE.WebGLRenderer({ antialias: true, devicePixelRatio: 1 });
            this.renderer.setSize(Constants.cameraWidth, Constants.cameraHeight, true);
            this.renderer.shadowMapEnabled = true;
            this.renderer.shadowMapSoft = true;

            this.updateCameraPosition();
            this.render();

            this.$element.append(this.renderer.domElement);

            this.bindMouseInteraction(this.renderer.domElement);
        };

        World3dModule.prototype.setBallPosition = function(x, y, z)
        {
            if (Math.abs(z) < 1e-3)
                z = Constants.ballRadius;

            this.ballMesh.position.set(x, y, z);
        };

        World3dModule.prototype.positionBodySpotlight = function(body)
        {
            var dist = 4,
                unit = Math.sqrt(dist*dist / 3);

            if (!body.spotlight)
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
                body.spotlight = light;
            }

            body.spotlight.position.set(body.position.x + unit, body.position.y + unit, body.position.z + unit);
            body.spotlight.target.position = body.position; //.set(0, 0, 0);
        };

        World3dModule.prototype.buildBody = function(body, loadedCallback)
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
                        // rotate to account for the different axes used in the json files
                        object.rotation.x = Math.PI/2;
                        object.rotation.y = Math.PI;
                        parentObject.add(object);

                        if (node.name) {
                            this.objectByName[node.name] = object;
                        }

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
                        childHinge.position.z = childNode.offset.z || 0;
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

        World3dModule.prototype.bindMouseInteraction = function(container)
        {
            var onMouseDownTheta = this.cameraTheta,
                onMouseDownPhi = this.cameraPhi;

            container.addEventListener('mousewheel', function(event)
            {
                if (!this.useThirdPerson)
                    return;
                event.preventDefault();
                this.cameraDistance *= 1 - (event.wheelDeltaY/720);
                this.cameraDistance = Math.max(0.1, Math.min(5, this.cameraDistance));
                this.updateCameraPosition();
                this.render();
            }.bind(this), false);

            Dragger.bind(container, {
                start: function()
                {
                    if (!this.useThirdPerson)
                        return;
                    onMouseDownTheta = this.cameraTheta;
                    onMouseDownPhi = this.cameraPhi;
                }.bind(this),
                move: function(dx, dy)
                {
                    if (!this.useThirdPerson)
                        return;
                    this.cameraTheta = -(dx * 0.01) + onMouseDownTheta;
                    this.cameraPhi = (dy * 0.01) + onMouseDownPhi;
                    this.cameraPhi = Math.min(Math.PI / 2, Math.max(-Math.PI / 2, this.cameraPhi));
                    this.updateCameraPosition();
                    this.render();
                }.bind(this)
            });
        };

        World3dModule.prototype.updateCameraPosition = function()
        {
            if (this.useThirdPerson) {
                // Third person -- position camera outside player
                var torsoPosition = this.bodyRoot ? this.bodyRoot.position : new THREE.Vector3(0, 0, 0);
                this.camera.position.x = this.cameraDistance * Math.sin(this.cameraTheta) * Math.cos(this.cameraPhi);
                this.camera.position.y = -this.cameraDistance * Math.cos(this.cameraTheta) * Math.cos(this.cameraPhi);
                this.camera.position.z = this.cameraDistance * Math.sin(this.cameraPhi);
                this.camera.position.add(torsoPosition);
                this.camera.up.set(0, 0, 1);
                this.camera.lookAt(torsoPosition);
            } else {
                // First person -- position camera in player's head
                var headMatrix = this.objectByName['head'].matrixWorld;
                this.camera.position = Constants.cameraOffsetInHead.clone().applyMatrix4(headMatrix);
                this.camera.lookAt(new THREE.Vector3(0, 0, 1).applyMatrix4(headMatrix));
            }
        };

        World3dModule.prototype.render = function()
        {
            // Only render once all textures are loaded
            if (this.pendingTextureCount !== 0)
                return;

            // Only render once per frame, regardless of how many times requested
            if (this.isRenderQueued)
                return;

            this.isRenderQueued = true;

            // Request render in the next frame
            requestAnimationFrame(function()
            {
                this.isRenderQueued = false;
                this.renderer.render(this.scene, this.camera);
            }.bind(this));
        };

        return World3dModule;
    }
);
