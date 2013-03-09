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
        // camera variables
        var cameraDistance = 0.4,
            cameraTheta = -Math.PI/4,
            cameraPhi = Math.PI/6;

        var camera, scene, renderer;

        init();

        // TODO get rid of keyboard control

        // use this for keyboard control
        var activeHingeIndex = 0;
        var hinges = [];

        var root = buildBody(Constants.bodyStructure, hinges, function()
        {
            scene.add(root);
            render();
        });

        function init()
        {
            scene = new THREE.Scene();
            scene.add(new THREE.AmbientLight(0x777777));

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
            scene.add(light);

            //
            // Ground
            //
            var groundTexture = THREE.ImageUtils.loadTexture("images/felt.jpg");
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
            scene.add(groundMesh);

            //
            // Ball
            //
            var ballMaterial = new THREE.MeshPhongMaterial({
                ambient: 0x666666,
                map: THREE.ImageUtils.loadTexture("images/ball-colour-map.png"),
                bumpMap: THREE.ImageUtils.loadTexture("images/ball-bump-map.jpg"),
                bumpScale: 0.0075
            } );

            var ballRadius = 0.037,
                ballSegments = 20,
                ballMesh = new THREE.Mesh(new THREE.SphereGeometry(ballRadius, ballSegments, ballSegments), ballMaterial);
            ballMesh.position.set(0.05, groundPlaneY + ballRadius, 0.1);
            ballMesh.castShadow = true;
            ballMesh.receiveShadow = false;
            scene.add(ballMesh);

            //
            // Render & Camera
            //
            var canvasWidth = 640, canvasHeight = 480;

            camera = new THREE.PerspectiveCamera( 75, canvasWidth / canvasHeight, 0.01, 100 );
//          camera = new THREE.OrthographicCamera(window.innerWidth / -2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / -2, 1, 1000);

            renderer = new THREE.WebGLRenderer({ antialias: true });
            renderer.setSize(canvasWidth, canvasHeight);
            renderer.shadowMapEnabled = true;
            renderer.shadowMapSoft = true;

            updateCameraPosition();

            var container = document.getElementById('model-container');
            container.appendChild(renderer.domElement);

            bindMouseInteraction(renderer.domElement);

            document.addEventListener('keydown', onDocumentKeyDown, false);
        }

        function buildBody(body, hinges, loadedCallback)
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
                        parentObject.add(object);
                        geometriesToLoad--;
                        if (geometriesToLoad === 0) {
                            loadedCallback();
                        }
                    });
                }

                for (var i = 0; node.children && i < node.children.length; i++) {
                    var childNode = node.children[i];
                    var childObject = new THREE.Object3D();
                    if (childNode.offset) {
                        childObject.position.x = childNode.offset.x || 0;
                        childObject.position.y = childNode.offset.y || 0;
                    }
                    childObject.rotationAxis = childNode.rotationAxis;
                    hinges[childNode.jointId] = childObject;
                    parentObject.add(childObject);
                    processNode(childNode, childObject);
                }
            };

            var root = new THREE.Object3D();
            processNode(body, root);

            return root;
        }


        function bindMouseInteraction(container)
        {
            var onMouseDownPosition = new THREE.Vector2(),
                onMouseDownTheta = cameraTheta,
                onMouseDownPhi = cameraPhi,
                isMouseDown = false;

            container.addEventListener('mousewheel', function(event)
            {
                event.preventDefault();
                cameraDistance *= 1 - (event.wheelDeltaY/720);
                cameraDistance = Math.max(0.1, Math.min(5, cameraDistance));
                updateCameraPosition();
            });

            container.addEventListener('mousedown', function(event)
            {
                event.preventDefault();
                isMouseDown = true;
                onMouseDownTheta = cameraTheta;
                onMouseDownPhi = cameraPhi;
                onMouseDownPosition.x = event.clientX;
                onMouseDownPosition.y = event.clientY;
            });

            container.addEventListener('mousemove', function(event)
            {
                event.preventDefault();
                if (isMouseDown) {
                    cameraTheta = -((event.clientX - onMouseDownPosition.x) * 0.01) + onMouseDownTheta;
                    cameraPhi = ((event.clientY - onMouseDownPosition.y) * 0.01) + onMouseDownPhi;
                    cameraPhi = Math.min(Math.PI/2, Math.max(-Math.PI/2, cameraPhi));
                    updateCameraPosition();
                }
            });

            container.addEventListener('mouseup', function(event)
            {
                event.preventDefault();
                isMouseDown = false;
                onMouseDownPosition.x = event.clientX - onMouseDownPosition.x;
                onMouseDownPosition.y = event.clientY - onMouseDownPosition.y;
            });
        }

        function onDocumentKeyDown(event)
        {
            switch (event.keyCode) {
                case 32: // space bar
                    event.preventDefault();
                    cycleActiveHinge();
                    break;
                case 37: // left arrow
                    event.preventDefault();
                    changeHingeAngle(-0.1);
                    break;
                case 39: // right arrow
                    event.preventDefault();
                    changeHingeAngle(0.1);
                    break;
            }
        }

        function cycleActiveHinge()
        {
            activeHingeIndex++;
            if (activeHingeIndex === hinges.length) {
                activeHingeIndex = 0;
            }
        }

        function changeHingeAngle(deltaRads)
        {
            var hinge = hinges[activeHingeIndex];
            hinge.rotation[hinge.rotationAxis] += deltaRads;
            render();
        }

        function updateCameraPosition()
        {
            camera.position.x = cameraDistance * Math.sin(cameraTheta) * Math.cos(cameraPhi);
            camera.position.y = cameraDistance * Math.sin(cameraPhi);
            camera.position.z = cameraDistance * Math.cos(cameraTheta) * Math.cos(cameraPhi);
            camera.lookAt(new THREE.Vector3(0,-0.1,0));
            render();
        }

        var isRenderQueued = false;
        function render()
        {
            // Only render once per frame, regardless of how many times requested
            if (isRenderQueued)
                return;
            isRenderQueued = true;
            requestAnimationFrame(function()
            {
                isRenderQueued = false;
                renderer.render(scene, camera);
            });
        }

        //
        // connect with streaming data
        //
        var subscription = DataProxy.subscribe(Protocols.agentModel, {
            onmessage: function (msg)
            {
                var floats = DataProxy.parseFloats(msg.data),
                    angles = floats.slice(6);

                if (!angles || angles.length !== 20)
                {
                    console.error("Expecting 20 angles");
                    return;
                }

                var hasChange = false;
                for (var i = 0; i < 20; i++)
                {
                    var hinge = hinges[i + 1];
                    if (hinge.rotation !== angles[i])
                    {
                        hinge.rotation[hinge.rotationAxis] = angles[i];
                        hasChange = true;
                    }
                }

                if (hasChange)
                {
                    render();
                }
            }
        });

        // TODO when we've moved to modules, wire up disposal of subscription
    }
);
