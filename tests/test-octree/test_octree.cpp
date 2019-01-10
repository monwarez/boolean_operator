
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <OpenMesh/Core/Geometry/Vector11T.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>



using testing::Eq;
using testing::FloatEq;

using MeshT = OpenMesh::TriMesh_ArrayKernelT<OpenMesh::DefaultTraits>;


#include <algorithm>
#include <array>
#include <map>
#include <vector>

#include "bbox.h"


template <typename MeshType>
class MeshFaceAndBBox
{
public:
    using FaceHandle = typename MeshType::FaceHandle;
    using Point      = typename MeshType::Point;

    MeshFaceAndBBox( MeshType& meshView )
        : mesh{meshView}
        , faceToBBox{computeFaceToBBox_()}
        , meshBBox{computeMeshBBox_()}
    {
    }

private:
    std::map<FaceHandle, BBox<Point>> computeFaceToBBox_() const
    {
        std::map<FaceHandle, BBox<Point>> faceToBBox;

        for ( auto fhIt = mesh.faces_begin(); fhIt != mesh.faces_end(); ++fhIt )
        {
            faceToBBox.emplace( *fhIt, bboxFromFace( mesh, *fhIt ) );
        }
        return faceToBBox;
    }

    BBox<Point> computeMeshBBox_() const
    {
        Point lower;
        Point upper;

        for ( int iDir = 0; iDir < 3; ++iDir )
        {
            auto min = std::min_element(
                std::begin( faceToBBox ), std::end( faceToBBox ),
                [&iDir]( auto const& lhs, auto const& rhs ) //
                { return lhs.second.lower()[ iDir ] < rhs.second.lower()[ iDir ]; } );

            auto max = std::max_element(
                std::begin( faceToBBox ), std::end( faceToBBox ),
                [&iDir]( auto const& lhs, auto const& rhs ) //
                { return lhs.second.upper()[ iDir ] < rhs.second.upper()[ iDir ]; } );

            lower[ iDir ] = min->second.lower()[ iDir ];
            upper[ iDir ] = max->second.upper()[ iDir ];
        }


        return BBox<Point>{lower, upper};
    }

public:
    MeshType const& mesh;
    std::map<FaceHandle, BBox<Point>> const faceToBBox;
    BBox<Point> const meshBBox;
};




TEST( bar, doesFoo )
{
    //

    MeshT mesh1;
    MeshT mesh2;

    std::array<MeshT::VertexHandle, 4> vhandleMesh1;
    std::array<MeshT::VertexHandle, 4> vhandleMesh2;

    vhandleMesh1[ 0 ] = mesh1.add_vertex( MeshT::Point{0., 0., 0.} );
    vhandleMesh1[ 1 ] = mesh1.add_vertex( MeshT::Point{4., 2., 0.} );
    vhandleMesh1[ 2 ] = mesh1.add_vertex( MeshT::Point{3., 6., 0.} );
    vhandleMesh1[ 3 ] = mesh1.add_vertex( MeshT::Point{0., 4., 0.} );


    vhandleMesh2[ 0 ] = mesh2.add_vertex( MeshT::Point{3., 1., 0.} );
    vhandleMesh2[ 1 ] = mesh2.add_vertex( MeshT::Point{5., 3., 0.} );
    vhandleMesh2[ 2 ] = mesh2.add_vertex( MeshT::Point{8., 4., 0.} );
    vhandleMesh2[ 3 ] = mesh2.add_vertex( MeshT::Point{0., 6., 0.} );

    std::vector<MeshT::VertexHandle> faceVhandles;
    faceVhandles.resize( 4 );
    std::copy( std::begin( vhandleMesh1 ), std::end( vhandleMesh1 ), std::begin( faceVhandles ) );

    mesh1.add_face( faceVhandles );

    std::copy( std::begin( vhandleMesh2 ), std::end( vhandleMesh2 ), std::begin( faceVhandles ) );

    mesh2.add_face( faceVhandles );




    using FaceHandle = typename MeshT::FaceHandle;

    using Point = typename MeshT::Point;

    auto faceAndBBox1 = MeshFaceAndBBox{mesh1};
    auto faceAndBBox2 = MeshFaceAndBBox{mesh2};

    auto const& mesh1box = faceAndBBox1.meshBBox;

    auto const& mesh2box = faceAndBBox2.meshBBox;

    EXPECT_DOUBLE_EQ( 0., mesh1box.lower()[ 0 ] );
    EXPECT_DOUBLE_EQ( 0., mesh1box.lower()[ 1 ] );
    EXPECT_DOUBLE_EQ( 0., mesh1box.lower()[ 2 ] );

    EXPECT_DOUBLE_EQ( 4., mesh1box.upper()[ 0 ] );
    EXPECT_DOUBLE_EQ( 6., mesh1box.upper()[ 1 ] );
    EXPECT_DOUBLE_EQ( 0., mesh1box.upper()[ 2 ] );


    ASSERT_TRUE( intersect( mesh1box, mesh2box ) );

    auto intersectionBox = bboxIntersect( mesh1box, mesh2box );
}
