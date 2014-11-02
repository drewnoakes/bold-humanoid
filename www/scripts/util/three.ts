/**
 * @author Drew Noakes http://drewnoakes.com
 */

/// <reference path="../../libs/three.d.ts" />

export function computeVertexNormals(geometry: THREE.Geometry, maxSmoothAngle: number)
{
    var v, vl, f, fl, face,
        faceIndicesPerVertex = [];

    for (v = 0, vl = geometry.vertices.length; v < vl; v++) {
        faceIndicesPerVertex.push([])
    }

    for (f = 0, fl = geometry.faces.length; f < fl; f++) {
        face = geometry.faces[f];

        faceIndicesPerVertex[face.a].push(f);
        faceIndicesPerVertex[face.b].push(f);
        faceIndicesPerVertex[face.c].push(f);
    }

    // for each face...

    for (f = 0, fl = geometry.faces.length; f < fl; f++) {

        face = geometry.faces[f];

        // for each vertex of the face...

        for (var fv = 0; fv < 3; fv++) {

            var vertexIndex = face['abcd'.charAt(fv)];
            var vertexFaces = faceIndicesPerVertex[vertexIndex];
            var vertexNormal = face.normal.clone();

            // for each neighbouring face that shares this vertex...

            for (var vf = 0; vf < vertexFaces.length; vf++) {

                var neighbourFaceIndex = vertexFaces[vf];
                var neighbourFace = geometry.faces[neighbourFaceIndex];

                // disregard the face we're working with
                if (neighbourFace === face)
                    continue;

                // given both normals are unit vectors, the angle is just acos(a.dot(b))
                var theta = Math.acos(face.normal.dot(neighbourFace.normal));

                if (theta <= maxSmoothAngle) {
                    vertexNormal.add(neighbourFace.normal);
                }
            }

            vertexNormal.normalize();

            face.vertexNormals[fv] = vertexNormal;
        }
    }
}
