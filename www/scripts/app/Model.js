/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
        'scripts/app/GeometryUtil'
    ],
    function(GeometryUtil)
    {
        // camera variables
        var cameraDistance = 0.4,
            cameraTheta = -Math.PI/4,
            cameraPhi = Math.PI/6;

        var camera, scene, renderer;

        init();

        // TODO define the offsets/rotations/etc below in terms of Constants
        // TODO map hinges below to servo IDs, and manipulate from agent-model protocol
        // TODO add forehead-camera geometry

        var body = {
            name: 'body',
            geometryPath: 'models/darwin/darwin-body.json',
            children: [
                {
                    name: 'neck',
                    geometryPath: 'models/darwin/darwin-neck.json',
                    offset: { x: 0, y: 0.051 },
                    rotationAxis: 'y',
                    children: [
                        {
                            name: 'head',
                            geometryPath: 'models/darwin/darwin-head.json',
                            creaseAngle: 0.52,
//                            offset: { x: 0, y: 0.0 },
                            rotationAxis: 'x', // TODO should be -1, 0, 0
                            children: [
                                {
                                    name: 'eye-led',
                                    creaseAngle: 0.52,
                                    geometryPath: 'models/darwin/darwin-eye-led.json',
                                    offset: { x: 0, y: 0 }
                                },
                                {
                                    name: 'forehead-led',
                                    geometryPath: 'models/darwin/darwin-forehead-led.json',
                                    offset: { x: 0, y: 0 }
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'pelvis-left-yaw',
                    geometryPath: 'models/darwin/darwin-pelvis-yaw-left.json',
                    offset: { x: 0.037, y: -0.1222, z: -0.005 },
                    rotationAxis: 'y', // TODO should be 0, -1, 0
                    children: [
                        {
                            name: 'pelvis-left',
                            geometryPath: 'models/darwin/darwin-pelvis-left.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: 'z', // TODO should be 0, 0, -1
                            children: [
                                {
                                    name: 'leg-upper-left',
                                    geometryPath: 'models/darwin/darwin-leg-upper-left.json',
                                    offset: { x: 0, y: 0, z: 0 },
                                    rotationAxis: 'x', // TODO should be -1, 0, 0
                                    children: [
                                        {
                                            name: 'leg-lower-left',
                                            geometryPath: 'models/darwin/darwin-leg-lower-left.json',
                                            offset: { x: 0, y: -0.093, z: 0 },
                                            rotationAxis: 'x', // TODO should be -1, 0, 0
                                            children: [
                                                {
                                                    name: 'ankle-left',
                                                    geometryPath: 'models/darwin/darwin-ankle-left.json',
                                                    offset: { x: 0, y: -0.093, z: 0 },
                                                    rotationAxis: 'x', // TODO should be 1, 0, 0
                                                    children: [
                                                        {
                                                            name: 'foot-left',
                                                            geometryPath: 'models/darwin/darwin-foot-left.json',
                                                            offset: { x: 0, y: 0, z: 0 },
                                                            rotationAxis: 'z' // TODO should be 0, 0, 1
                                                        }
                                                    ]
                                                }
                                            ]
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'pelvis-right-yaw',
                    geometryPath: 'models/darwin/darwin-pelvis-yaw-right.json',
                    offset: { x: -0.037, y: -0.1222, z: -0.005 },
                    rotationAxis: 'y', // TODO should be 0, -1, 0
                    children: [
                        {
                            name: 'pelvis-right',
                            geometryPath: 'models/darwin/darwin-pelvis-right.json',
                            offset: { x: 0, y: 0, z: 0 },
                            rotationAxis: 'z', // TODO should be 0, 0, -1
                            children: [
                                {
                                    name: 'leg-upper-right',
                                    geometryPath: 'models/darwin/darwin-leg-upper-right.json',
                                    offset: { x: 0, y: 0, z: 0 },
                                    rotationAxis: 'x', // TODO should be 1, 0, 0
                                    children: [
                                        {
                                            name: 'leg-lower-right',
                                            geometryPath: 'models/darwin/darwin-leg-lower-right.json',
                                            offset: { x: 0, y: -0.093, z: 0 },
                                            rotationAxis: 'x', // TODO should be 1, 0, 0
                                            children: [
                                                {
                                                    name: 'ankle-right',
                                                    geometryPath: 'models/darwin/darwin-ankle-right.json',
                                                    offset: { x: 0, y: -0.093, z: 0 },
                                                    rotationAxis: 'x', // TODO should be -1, 0, 0
                                                    children: [
                                                        {
                                                            name: 'foot-right',
                                                            geometryPath: 'models/darwin/darwin-foot-right.json',
                                                            offset: { x: 0, y: 0, z: 0 },
                                                            rotationAxis: 'z' // TODO should be 0, 0, 1
                                                        }
                                                    ]
                                                }
                                            ]
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'shoulder-left',
                    geometryPath: 'models/darwin/darwin-shoulder-left.json',
                    offset: { x: 0.082, y: 0, z: 0 },
                    rotationAxis: 'x', // 1 0 0
                    children: [
                        {
                            name: 'arm-upper-left',
                            geometryPath: 'models/darwin/darwin-arm-upper-left.json',
                            offset: { x: 0, y: -0.016, z: 0 },
                            rotationAxis: 'z', // TODO should be 0 0 -1 -0.7854
                            children: [
                                {
                                    name: 'arm-lower-left',
                                    geometryPath: 'models/darwin/darwin-arm-lower-left.json',
                                    offset: { x: 0, y: -0.06, z: 0.016 },
                                    rotationAxis: 'x' // TODO should be 1 0 0 -1.5708
                                }
                            ]
                        }
                    ]
                },
                {
                    name: 'shoulder-right',
                    geometryPath: 'models/darwin/darwin-shoulder-right.json',
                    offset: { x: -0.082, y: 0, z: 0 },
                    rotationAxis: 'x', // -1 0 0
                    children: [
                        {
                            name: 'arm-upper-right',
                            geometryPath: 'models/darwin/darwin-arm-upper-right.json',
                            offset: { x: 0, y: -0.016, z: 0 },
                            rotationAxis: 'z', // TODO should be 0 0 -1 0.7854
                            children: [
                                {
                                    name: 'arm-lower-right',
                                    geometryPath: 'models/darwin/darwin-arm-lower-right.json',
                                    offset: { x: 0, y: -0.06, z: 0.016 },
                                    rotationAxis: 'x' // TODO should be -1 0 0 1.5708
                                }
                            ]
                        }
                    ]
                }
            ]
        };

        // use this for keyboard control
        var activeHingeIndex = 0;
        var hinges = [];

        var root = buildBody(body, hinges, function()
        {
            scene.add(root);
            render();
        });

        function init()
        {
            scene = new THREE.Scene();
            scene.add(new THREE.AmbientLight(0x555555));

            var light = new THREE.DirectionalLight(0xffffff);
            light.position.set(0, 0, 1);
            light.position.normalize();
            scene.add(light);

//            light = new THREE.DirectionalLight(0xffffff);
//            light.position.set(0, 1, 0);
//            light.position.normalize();
//            scene.add(light);

            light = new THREE.PointLight(0xffffff);
            light.position.set(0, 1, -1);
            light.position.normalize();
            scene.add(light);

            renderer = new THREE.WebGLRenderer({ antialias: true });
            renderer.setSize(window.innerWidth, window.innerHeight);

            camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.01, 100000 );
//          camera = new THREE.OrthographicCamera(window.innerWidth / -2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / -2, 1, 1000);

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
                        parentObject.add(new THREE.Mesh(geometry, new THREE.MeshFaceMaterial( materials )));
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
                    hinges.push(childObject);
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

        function render()
        {
            requestAnimationFrame(function() { renderer.render(scene, camera); });
        }
    }
);
