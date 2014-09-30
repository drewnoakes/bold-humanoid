/**
 * @author Drew Noakes https://drewnoakes.com
 */

import net = require('util/Network');

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

export function withDarwinModel(callback: IPartMapCallback)
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
            'models/darwin.json',
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
