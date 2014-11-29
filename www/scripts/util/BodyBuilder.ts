/**
 * @author Drew Noakes https://drewnoakes.com
 */

import net = require('util/Network');
import constants = require('constants');
import GeometryUtil = require('util/three');

export interface IMaterial
{
    DbgName: string;
    blending: string;
    colorAmbient: number[];
    colorDiffuse: number[];
    colorSpecular: number[];
    shading: string;
    specularCoef: number;
    transparency: number;
    transparent: boolean;
    vertexColors: boolean;
}

export interface IPartModel
{
    scale: number;
    metadata?: Object;
    materials: IMaterial[];
    vertices: number[];
    faces: number[];
    // NOTE some other stuff omitted
}

export interface IPartMap { [partName: string]: IPartModel; }
export interface IPartMapCallback { (parts: IPartMap): void; }

var parts: IPartMap;
var isRequestPending = false;
var callbacks: IPartMapCallback[] = [];

function withDarwinModel(callback: IPartMapCallback)
{
    if (parts)
    {
        callback(parts);
        return;
    }

    if (isRequestPending)
    {
        callbacks.push(callback);
    }
    else
    {
        isRequestPending = true;
        net.load<IPartMap>(
            'resources/darwin.json',
            partMap => {
                parts = partMap;
                isRequestPending = false;
                callback(partMap);
                for (var i = 0; i < callbacks.length; i++)
                    callbacks[i](partMap);
                callbacks = [];
            },
            xhr => {
                console.error("Unable to load DARwIn-OP model data");
                console.dir(xhr);
                isRequestPending = false;
            }
        );
    }
}

export interface XYZ
{
    x:number; y:number; z:number;
}

export class Hinge
{
    public rotationAxis: THREE.Euler;
    public rotationOrigin: number;
    public childObject: THREE.Object3D;

    constructor(parentPart: constants.IBodyPart)
    {
        this.childObject = new THREE.Object3D();

        if (parentPart.offset) {
            this.childObject.position.x = parentPart.offset.x || 0;
            this.childObject.position.y = parentPart.offset.y || 0;
            this.childObject.position.z = parentPart.offset.z || 0;
        }

        this.rotationAxis = parentPart.rotationAxis;
        this.rotationOrigin = parentPart.rotationOrigin;

        this.setAngle(0);
    }

    public setAngle(angleRads: number)
    {
        if (!this.rotationAxis) {
            // No hinge is defined (eg: eyes)
            return false;
        }

        // Add any angular offset applied to this hinge (adjust the zero-position)
        if (this.rotationOrigin) {
            angleRads += this.rotationOrigin;
        }

        var rotation = new THREE.Euler(
            this.rotationAxis.x * angleRads,
            this.rotationAxis.y * angleRads,
            this.rotationAxis.z * angleRads
        );

        if (!this.childObject.rotation.equals(rotation)) {
            this.childObject.rotation.set(rotation.x, rotation.y, rotation.z);
            return true;
        }

        // No change was applied
        return false;
    }
}

export function buildBody(hinges: {[id:number]:Hinge}, objectByName: {[name:string]:THREE.Mesh}, loadedCallback: ()=>void)
{
    console.assert(!!hinges && !!objectByName);

    var processNode = (node: constants.IBodyPart, parentObject: THREE.Object3D, partMap: IPartMap) =>
    {
        console.assert(!!node.name);

        var loader = new THREE.JSONLoader();
        var model = loader.parse(<any>partMap[node.name]);
        model.geometry.computeFaceNormals();
//      geometry.computeVertexNormals();
        GeometryUtil.computeVertexNormals(model.geometry, node.creaseAngle || 0.2);

        var object = new THREE.Mesh(model.geometry, new THREE.MeshFaceMaterial(model.materials));
        object.castShadow = true;
        object.receiveShadow  = false;
        // rotate to account for the different axes used in the json files
        object.rotation.x = Math.PI/2;
        object.rotation.y = Math.PI;
        parentObject.add(object);

        objectByName[node.name] = object;

        for (var i = 0; node.children && i < node.children.length; i++) {
            // Create a hinge for each child object
            var childPart = node.children[i];
            var hinge = new Hinge(childPart);
            hinges[childPart.jointId] = hinge;
            parentObject.add(hinge.childObject);
            processNode(childPart, hinge.childObject, partMap);
        }
    };

    var root = new THREE.Object3D();

    withDarwinModel(partMap =>
    {
        // Start with the root
        processNode(constants.bodyStructure, root, partMap);

        // Callback to signal we're complete
        loadedCallback();
    });

    // Return the root which will be populated once the JSON has been loaded (or immediately if already cached).
    return root;
}
