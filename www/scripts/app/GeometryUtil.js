/**
 * @author Drew Noakes http://drewnoakes.com
 */
define(
    [
    ],
    function ()
    {
        'use strict';

        // noinspection UnnecessaryLocalVariableJS

        var GeometryUtil = {

            computeVertexNormals: function(geometry, maxSmoothAngle)
            {

                var v, vl, f, fl, face,
                    faceIndicesPerVertex = [];

                for (v = 0, vl = geometry.vertices.length; v < vl; v ++) {

                    faceIndicesPerVertex.push([])

                }

                for (f = 0, fl = geometry.faces.length; f < fl; f ++) {

                    face = geometry.faces[ f ];

                    if ( face instanceof THREE.Face3 ) {

                        faceIndicesPerVertex[face.a].push(f);
                        faceIndicesPerVertex[face.b].push(f);
                        faceIndicesPerVertex[face.c].push(f);

                    } else if ( face instanceof THREE.Face4 ) {

                        faceIndicesPerVertex[face.a].push(f);
                        faceIndicesPerVertex[face.b].push(f);
                        faceIndicesPerVertex[face.c].push(f);
                        faceIndicesPerVertex[face.d].push(f);

                    }

                }

				// for each face...

                for (f = 0, fl = geometry.faces.length; f < fl; f ++) {

                    face = geometry.faces[ f ];

                    var faceVertexCount;
                    if ( face instanceof THREE.Face3 )
                        faceVertexCount = 3;
                    else if ( face instanceof THREE.Face4 )
                        faceVertexCount = 4;
                    else
                        continue;

					// for each vertex of the face...

					for ( var fv = 0; fv < faceVertexCount; fv ++ ) {

						var vertexIndex = face[ 'abcd'.charAt( fv ) ];

						var vertexFaces = faceIndicesPerVertex[ vertexIndex ];

						var vertexNormal = face.normal.clone();

						// for each neighbouring face that shares this vertex...

						for ( var vf = 0; vf < vertexFaces.length; vf ++ ) {

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

						face.vertexNormals[ fv ] = vertexNormal;
                    }

                }

			}

        };

        return GeometryUtil;
    }
);