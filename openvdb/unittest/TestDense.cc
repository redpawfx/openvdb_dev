///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012-2013 DreamWorks Animation LLC
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// *     Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////

//#define BENCHMARK_TEST

#include <openvdb/openvdb.h>
#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/Dense.h>
#include <openvdb/Exceptions.h>
#include <sstream>
#ifdef BENCHMARK_TEST
#include "util.h" // for CpuTimer
#endif


class TestDense: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestDense);
    CPPUNIT_TEST(testDense);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testCopyBool);
    CPPUNIT_TEST(testDense2Sparse);
    CPPUNIT_TEST(testDense2Sparse2);
    CPPUNIT_TEST(testInvalidBBox);
    CPPUNIT_TEST(testDense2Sparse2Dense);
    CPPUNIT_TEST_SUITE_END();

    void testDense();
    void testCopy();
    void testCopyBool();
    void testDense2Sparse();
    void testDense2Sparse2();
    void testInvalidBBox();
    void testDense2Sparse2Dense();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDense);


void
TestDense::testDense()
{
    const openvdb::CoordBBox bbox(openvdb::Coord(-40,-5, 6),
                                  openvdb::Coord(-11, 7,22));
    openvdb::tools::Dense<float> dense(bbox);

    // Check Dense::valueCount
    const int size = dense.valueCount();
    CPPUNIT_ASSERT_EQUAL(30*13*17, size);

    // Cehck Dense::fill(float) and Dense::getValue(size_t)
    const float v = 0.234f;
    dense.fill(v);
    for (int i=0; i<size; ++i) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(v, dense.getValue(i),/*tolerance=*/0.0001);
    }

    // Check Dense::data() and Dense::getValue(Coord, float)
    float* a = dense.data();
    int s = size;
    while(s--) CPPUNIT_ASSERT_DOUBLES_EQUAL(v, *a++, /*tolerance=*/0.0001);

    for (openvdb::Coord P(bbox.min()); P[0] <= bbox.max()[0]; ++P[0]) {
        for (P[1] = bbox.min()[1]; P[1] <= bbox.max()[1]; ++P[1]) {
            for (P[2] = bbox.min()[2]; P[2] <= bbox.max()[2]; ++P[2]) {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(v, dense.getValue(P), /*tolerance=*/0.0001);
            }
        }
    }

    // Check Dense::setValue(Coord, float)
    const openvdb::Coord C(-30, 3,12);
    const float v1 = 3.45f;
    dense.setValue(C, v1);
    for (openvdb::Coord P(bbox.min()); P[0] <= bbox.max()[0]; ++P[0]) {
        for (P[1] = bbox.min()[1]; P[1] <= bbox.max()[1]; ++P[1]) {
            for (P[2] = bbox.min()[2]; P[2] <= bbox.max()[2]; ++P[2]) {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(P==C ? v1 : v, dense.getValue(P),
                    /*tolerance=*/0.0001);
            }
        }
    }

    // Check Dense::setValue(size_t, size_t, size_t, float)
    dense.setValue(C, v);
    const openvdb::Coord L(1,2,3), C1 = bbox.min() + L;
    dense.setValue(L[0], L[1], L[2], v1);
    for (openvdb::Coord P(bbox.min()); P[0] <= bbox.max()[0]; ++P[0]) {
        for (P[1] = bbox.min()[1]; P[1] <= bbox.max()[1]; ++P[1]) {
            for (P[2] = bbox.min()[2]; P[2] <= bbox.max()[2]; ++P[2]) {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(P==C1 ? v1 : v, dense.getValue(P),
                    /*tolerance=*/0.0001);
            }
        }
    }

}


// The check is so slow that we're going to multi-thread it :)
template <typename TreeT>
class CheckDense
{
public:
    typedef typename TreeT::ValueType     ValueT;
    typedef openvdb::tools::Dense<ValueT> DenseT;

    CheckDense() : mTree(NULL), mDense(NULL) {}

    void check(const TreeT& tree, const DenseT& dense)
    {
        mTree  = &tree;
        mDense = &dense;
        tbb::parallel_for(dense.bbox(), *this);
    }
    void operator()(const openvdb::CoordBBox& bbox) const
    {
        openvdb::tree::ValueAccessor<const TreeT> acc(*mTree);
        for (openvdb::Coord P(bbox.min()); P[0] <= bbox.max()[0]; ++P[0]) {
            for (P[1] = bbox.min()[1]; P[1] <= bbox.max()[1]; ++P[1]) {
                for (P[2] = bbox.min()[2]; P[2] <= bbox.max()[2]; ++P[2]) {
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(acc.getValue(P), mDense->getValue(P),
                        /*tolerance=*/0.0001);
                }
            }
        }
    }
private:
    const TreeT*  mTree;
    const DenseT* mDense;
};// CheckDense


void
TestDense::testCopy()
{
    CheckDense<openvdb::FloatTree> checkDense;
    const float radius = 10.0f, tolerance = 0.00001f;
    const openvdb::Vec3f center(0.0f);
    // decrease the voxelSize to test larger grids
#ifdef BENCHMARK_TEST
    const float voxelSize = 0.05f, width = 5.0f;
#else
    const float voxelSize = 0.2f, width = 5.0f;
#endif

    // Create a VDB containing a level set of a sphere
    openvdb::FloatGrid::Ptr grid =
        openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(radius, center, voxelSize, width);
    openvdb::FloatTree& tree0 = grid->tree();

    // Create an empty dense grid
    openvdb::tools::Dense<float> dense(grid->evalActiveVoxelBoundingBox());
#ifdef BENCHMARK_TEST
    std::cerr << "\nBBox = " << grid->evalActiveVoxelBoundingBox() << std::endl;
#endif

    {//check Dense::fill
        dense.fill(voxelSize);
#ifndef BENCHMARK_TEST
        checkDense.check(openvdb::FloatTree(voxelSize), dense);
#endif
    }

    {// parallel convert to dense
#ifdef BENCHMARK_TEST
        unittest_util::CpuTimer ts;
        ts.start("CopyToDense");
#endif
        openvdb::tools::copyToDense(*grid, dense);
#ifdef BENCHMARK_TEST
        ts.stop();
#else
        checkDense.check(tree0, dense);
#endif
    }

    {// Parallel create from dense
#ifdef BENCHMARK_TEST
        unittest_util::CpuTimer ts;
        ts.start("CopyFromDense");
#endif
        openvdb::FloatTree tree1(tree0.background());
        openvdb::tools::copyFromDense(dense, tree1, tolerance);
#ifdef BENCHMARK_TEST
        ts.stop();
#else
        checkDense.check(tree1, dense);
#endif
    }
}


void
TestDense::testCopyBool()
{
    using namespace openvdb;

    const Coord bmin(-1), bmax(8);
    const CoordBBox bbox(bmin, bmax);

    BoolGrid::Ptr grid = createGrid<BoolGrid>(false);
    BoolGrid::ConstAccessor acc = grid->getConstAccessor();

    tools::Dense<bool> dense(bbox);
    dense.fill(false);

    // Start with sparse and dense grids both filled with false.
    Coord xyz;
    int &x = xyz[0], &y = xyz[1], &z = xyz[2];
    for (x = bmin.x(); x <= bmax.x(); ++x) {
        for (y = bmin.y(); y <= bmax.y(); ++y) {
            for (z = bmin.z(); z <= bmax.z(); ++z) {
                CPPUNIT_ASSERT_EQUAL(false, dense.getValue(xyz));
                CPPUNIT_ASSERT_EQUAL(false, acc.getValue(xyz));
            }
        }
    }

    // Fill the dense grid with true.
    dense.fill(true);
    // Copy the contents of the dense grid to the sparse grid.
    openvdb::tools::copyFromDense(dense, *grid, /*tolerance=*/false);

    // Verify that both sparse and dense grids are now filled with true.
    for (x = bmin.x(); x <= bmax.x(); ++x) {
        for (y = bmin.y(); y <= bmax.y(); ++y) {
            for (z = bmin.z(); z <= bmax.z(); ++z) {
                CPPUNIT_ASSERT_EQUAL(true, dense.getValue(xyz));
                CPPUNIT_ASSERT_EQUAL(true, acc.getValue(xyz));
            }
        }
    }

    // Fill the dense grid with false.
    dense.fill(false);
    // Copy the contents (= true) of the sparse grid to the dense grid.
    openvdb::tools::copyToDense(*grid, dense);

    // Verify that the dense grid is now filled with true.
    for (x = bmin.x(); x <= bmax.x(); ++x) {
        for (y = bmin.y(); y <= bmax.y(); ++y) {
            for (z = bmin.z(); z <= bmax.z(); ++z) {
                CPPUNIT_ASSERT_EQUAL(true, dense.getValue(xyz));
            }
        }
    }
}

void
TestDense::testDense2Sparse()
{
    // The following test revealed a bug in v2.0.0b2
    
    // Test Domain Resolution
    size_t sizeX = 8;
    size_t sizeY = 8;
    size_t sizeZ = 9;

    // Define a dense grid
    openvdb::tools::Dense<float> dense(openvdb::Coord(sizeX, sizeY, sizeZ));
    const openvdb::CoordBBox bboxD = dense.bbox();
    // std::cerr <<  "\nDense bbox" << bboxD << std::endl;

    // Verify that the CoordBBox is truely used as [inclusive, inclusive] 
    CPPUNIT_ASSERT(dense.valueCount() == sizeX * sizeY * sizeZ );

    // Fill the dense grid with constant value 1.
    dense.fill(1.0f);

    // Create two empty float grids
    openvdb::FloatGrid::Ptr gridS = openvdb::FloatGrid::create(0.0f /*background*/);
    openvdb::FloatGrid::Ptr gridP = openvdb::FloatGrid::create(0.0f /*background*/);
    
    // Convert in serial and parallel modes
    openvdb::tools::copyFromDense(dense, *gridS, /*tolerance*/0.0f, /*serial = */ true);
    openvdb::tools::copyFromDense(dense, *gridP, /*tolerance*/0.0f, /*serial = */ false);
    
    float minS, maxS;
    float minP, maxP;
    
    gridS->evalMinMax(minS, maxS);
    gridP->evalMinMax(minP, maxP);
    
    const float tolerance = 0.0001;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(minS, minP, tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(maxS, maxP, tolerance);
    CPPUNIT_ASSERT_EQUAL(gridP->activeVoxelCount(), openvdb::Index64(sizeX * sizeY * sizeZ));

    const openvdb::FloatTree& treeS = gridS->tree();
    const openvdb::FloatTree& treeP = gridP->tree();
   
    // Values in Test Domain are correct
    for (openvdb::Coord ijk(bboxD.min()); ijk[0] <= bboxD.max()[0]; ++ijk[0]) {
        for (ijk[1] = bboxD.min()[1]; ijk[1] <= bboxD.max()[1]; ++ijk[1]) {
            for (ijk[2] = bboxD.min()[2]; ijk[2] <= bboxD.max()[2]; ++ijk[2]) {
                
                const float expected = bboxD.isInside(ijk) ? 1.f : 0.f; 
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, 1.f, tolerance);
                
                const float& vS = treeS.getValue(ijk);
                const float& vP = treeP.getValue(ijk);
                
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vS, tolerance);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vP, tolerance);
            }
        }
    }

    openvdb::CoordBBox bboxP = gridP->evalActiveVoxelBoundingBox();
    const openvdb::Index64 voxelCountP = gridP->activeVoxelCount();
    //std::cerr <<  "\nParallel: bbox=" << bboxP << " voxels=" << voxelCountP << std::endl;
    CPPUNIT_ASSERT( bboxP == bboxD );
    CPPUNIT_ASSERT_EQUAL( dense.valueCount(), voxelCountP);
    
    openvdb::CoordBBox bboxS = gridS->evalActiveVoxelBoundingBox();
    const openvdb::Index64 voxelCountS = gridS->activeVoxelCount();
    //std::cerr <<  "\nSerial: bbox=" << bboxS << " voxels=" << voxelCountS << std::endl;
    CPPUNIT_ASSERT( bboxS == bboxD );
    CPPUNIT_ASSERT_EQUAL( dense.valueCount(), voxelCountS);
    
    // Topology
    CPPUNIT_ASSERT( bboxS.isInside(bboxS) );
    CPPUNIT_ASSERT( bboxP.isInside(bboxP) );
    CPPUNIT_ASSERT( bboxS.isInside(bboxP) );
    CPPUNIT_ASSERT( bboxP.isInside(bboxS) );

    /// Check that the two grids agree
    for (openvdb::Coord ijk(bboxS.min()); ijk[0] <= bboxS.max()[0]; ++ijk[0]) {
        for (ijk[1] = bboxS.min()[1]; ijk[1] <= bboxS.max()[1]; ++ijk[1]) {
            for (ijk[2] = bboxS.min()[2]; ijk[2] <= bboxS.max()[2]; ++ijk[2]) {
                
                const float& vS = treeS.getValue(ijk);
                const float& vP = treeP.getValue(ijk);
              
                CPPUNIT_ASSERT_DOUBLES_EQUAL(vS, vP, tolerance); 

                // the value we should get based on the original domain 
                const float expected = bboxD.isInside(ijk) ? 1.f : 0.f; 
                
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vP, tolerance);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vS, tolerance);
            }
        }
    }


    // Verify the tree topology matches.
   
    CPPUNIT_ASSERT_EQUAL(gridP->activeVoxelCount(), gridS->activeVoxelCount());
    CPPUNIT_ASSERT(gridP->evalActiveVoxelBoundingBox() == gridS->evalActiveVoxelBoundingBox()); 
    CPPUNIT_ASSERT(treeP.hasSameTopology(treeS) ); 
    
}

void
TestDense::testDense2Sparse2()
{
    // The following tests copying a dense grid into a VDB tree with
    // existing values outside the bbox of the dense grid.
    
    // Test Domain Resolution
    size_t sizeX = 8;
    size_t sizeY = 8;
    size_t sizeZ = 9;
    const openvdb::Coord magicVoxel(sizeX, sizeY, sizeZ);

    // Define a dense grid
    openvdb::tools::Dense<float> dense(openvdb::Coord(sizeX, sizeY, sizeZ));
    const openvdb::CoordBBox bboxD = dense.bbox();
    //std::cerr <<  "\nDense bbox" << bboxD << std::endl;

    // Verify that the CoordBBox is truely used as [inclusive, inclusive] 
    CPPUNIT_ASSERT(dense.valueCount() == sizeX * sizeY * sizeZ );

    // Fill the dense grid with constant value 1.
    dense.fill(1.0f);

    // Create two empty float grids
    openvdb::FloatGrid::Ptr gridS = openvdb::FloatGrid::create(0.0f /*background*/);
    openvdb::FloatGrid::Ptr gridP = openvdb::FloatGrid::create(0.0f /*background*/);
    gridS->tree().setValue(magicVoxel, 5.0f);
    gridP->tree().setValue(magicVoxel, 5.0f);
    
    // Convert in serial and parallel modes
    openvdb::tools::copyFromDense(dense, *gridS, /*tolerance*/0.0f, /*serial = */ true);
    openvdb::tools::copyFromDense(dense, *gridP, /*tolerance*/0.0f, /*serial = */ false);
    
    float minS, maxS;
    float minP, maxP;
    
    gridS->evalMinMax(minS, maxS);
    gridP->evalMinMax(minP, maxP);
    
    const float tolerance = 0.0001;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, minP, tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, minS, tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, maxP, tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0f, maxS, tolerance);
    CPPUNIT_ASSERT_EQUAL(gridP->activeVoxelCount(), openvdb::Index64(1 + sizeX * sizeY * sizeZ));

    const openvdb::FloatTree& treeS = gridS->tree();
    const openvdb::FloatTree& treeP = gridP->tree();
   
    // Values in Test Domain are correct
    for (openvdb::Coord ijk(bboxD.min()); ijk[0] <= bboxD.max()[0]; ++ijk[0]) {
        for (ijk[1] = bboxD.min()[1]; ijk[1] <= bboxD.max()[1]; ++ijk[1]) {
            for (ijk[2] = bboxD.min()[2]; ijk[2] <= bboxD.max()[2]; ++ijk[2]) {
                
                const float expected = bboxD.isInside(ijk) ? 1.0f : 0.0f; 
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, 1.0f, tolerance);
                
                const float& vS = treeS.getValue(ijk);
                const float& vP = treeP.getValue(ijk);
                
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vS, tolerance);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vP, tolerance);
            }
        }
    }

    openvdb::CoordBBox bboxP = gridP->evalActiveVoxelBoundingBox();
    const openvdb::Index64 voxelCountP = gridP->activeVoxelCount();
    //std::cerr <<  "\nParallel: bbox=" << bboxP << " voxels=" << voxelCountP << std::endl;
    CPPUNIT_ASSERT( bboxP != bboxD );
    CPPUNIT_ASSERT( bboxP == openvdb::CoordBBox(openvdb::Coord(0,0,0), magicVoxel) );
    CPPUNIT_ASSERT_EQUAL( dense.valueCount()+1, voxelCountP);
    
    openvdb::CoordBBox bboxS = gridS->evalActiveVoxelBoundingBox();
    const openvdb::Index64 voxelCountS = gridS->activeVoxelCount();
    //std::cerr <<  "\nSerial: bbox=" << bboxS << " voxels=" << voxelCountS << std::endl;
    CPPUNIT_ASSERT( bboxS != bboxD );
    CPPUNIT_ASSERT( bboxS == openvdb::CoordBBox(openvdb::Coord(0,0,0), magicVoxel) );
    CPPUNIT_ASSERT_EQUAL( dense.valueCount()+1, voxelCountS);
    
    // Topology
    CPPUNIT_ASSERT( bboxS.isInside(bboxS) );
    CPPUNIT_ASSERT( bboxP.isInside(bboxP) );
    CPPUNIT_ASSERT( bboxS.isInside(bboxP) );
    CPPUNIT_ASSERT( bboxP.isInside(bboxS) );

    /// Check that the two grids agree
    for (openvdb::Coord ijk(bboxS.min()); ijk[0] <= bboxS.max()[0]; ++ijk[0]) {
        for (ijk[1] = bboxS.min()[1]; ijk[1] <= bboxS.max()[1]; ++ijk[1]) {
            for (ijk[2] = bboxS.min()[2]; ijk[2] <= bboxS.max()[2]; ++ijk[2]) {
                
                const float& vS = treeS.getValue(ijk);
                const float& vP = treeP.getValue(ijk);
              
                CPPUNIT_ASSERT_DOUBLES_EQUAL(vS, vP, tolerance); 

                // the value we should get based on the original domain 
                const float expected = bboxD.isInside(ijk) ? 1.0f
                    : ijk == magicVoxel ? 5.0f : 0.0f; 
                
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vP, tolerance);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, vS, tolerance);
            }
        }
    }

    // Verify the tree topology matches.
   
    CPPUNIT_ASSERT_EQUAL(gridP->activeVoxelCount(), gridS->activeVoxelCount());
    CPPUNIT_ASSERT(gridP->evalActiveVoxelBoundingBox() == gridS->evalActiveVoxelBoundingBox()); 
    CPPUNIT_ASSERT(treeP.hasSameTopology(treeS) ); 
    
}


void 
TestDense::testInvalidBBox() 
{
    const openvdb::CoordBBox badBBox(openvdb::Coord(1, 1, 1), openvdb::Coord(-1, 2, 2));
    
    CPPUNIT_ASSERT( badBBox.empty() );
    
    try {
        
        openvdb::tools::Dense<float> dense(badBBox);
 
    } catch (openvdb::ValueError&) {
        CPPUNIT_ASSERT( 1 );  // Caught the correct error
    } catch (std::bad_alloc& ) {
        
        CPPUNIT_ASSERT( 0 ); // Failed

        // currently this fails because CoordBBox::volume() returns an Index64
    }
        
}

void
TestDense::testDense2Sparse2Dense()
{
    const openvdb::CoordBBox bboxBig(openvdb::Coord(-12, 7, -32), openvdb::Coord(12, 14, -15));
    const openvdb::CoordBBox bboxSmall(openvdb::Coord(-10, 8, -31), openvdb::Coord(10, 12, -20));

    
    // A larger bbox 
    openvdb::CoordBBox bboxBigger = bboxBig;
    bboxBigger.expand(openvdb::Coord(10));

    
    // Small is in big
    CPPUNIT_ASSERT(bboxBig.isInside(bboxSmall));

    // Big is in Bigger
    CPPUNIT_ASSERT(bboxBigger.isInside(bboxBig));

    // Construct a small dense grid
    openvdb::tools::Dense<float> denseSmall(bboxSmall, 0.f);
    {
        // insert non-const values 
        const int n = denseSmall.valueCount();
        float* d = denseSmall.data();
        for (int i = 0; i < n; ++i) { d[i] = i; }
    }
    // Construct large dense grid
    openvdb::tools::Dense<float> denseBig(bboxBig, 0.f);
    {
        // insert non-const values 
        const int n = denseBig.valueCount();
        float* d = denseBig.data();
        for (int i = 0; i < n; ++i) { d[i] = i; }
    }

    // Make a sparse grid to copy this data into
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(3.3f /*background*/);
    openvdb::tools::copyFromDense(denseBig, *grid, /*tolerance*/0.0f, /*serial = */ true);
    openvdb::tools::copyFromDense(denseSmall, *grid, /*tolerance*/0.0f, /*serial = */ false);
      
    const openvdb::FloatTree& tree = grid->tree();
    // 
    CPPUNIT_ASSERT_EQUAL(bboxBig.volume(), grid->activeVoxelCount());
    
    // iterate over the Bigger
    for (openvdb::Coord ijk(bboxBigger.min()); ijk[0] <= bboxBigger.max()[0]; ++ijk[0]) {
        for (ijk[1] = bboxBigger.min()[1]; ijk[1] <= bboxBigger.max()[1]; ++ijk[1]) {
            for (ijk[2] = bboxBigger.min()[2]; ijk[2] <= bboxBigger.max()[2]; ++ijk[2]) {
                
                float expected = 3.3f;
                if (bboxSmall.isInside(ijk)) {
                    expected = denseSmall.getValue(ijk);
                } else if (bboxBig.isInside(ijk)) {
                    expected = denseBig.getValue(ijk);
                }
                
                const float& value = tree.getValue(ijk); 
                
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, value, 0.0001);
                
            }
        }
    }    

    // Convert to Dense in small bbox
    {
        openvdb::tools::Dense<float> denseSmall2(bboxSmall);
        openvdb::tools::copyToDense(*grid, denseSmall2, true /* serial */);
        
        // iterate over the Bigger
        for (openvdb::Coord ijk(bboxSmall.min()); ijk[0] <= bboxSmall.max()[0]; ++ijk[0]) {
            for (ijk[1] = bboxSmall.min()[1]; ijk[1] <= bboxSmall.max()[1]; ++ijk[1]) {
                for (ijk[2] = bboxSmall.min()[2]; ijk[2] <= bboxSmall.max()[2]; ++ijk[2]) {
                    
                    const float& expected = denseSmall.getValue(ijk);
                    const float& value = denseSmall2.getValue(ijk);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, value, 0.0001);
                }
            }
        }
    }
    // Convert to Dense in large bbox
    {
        openvdb::tools::Dense<float> denseBig2(bboxBig);
        
        openvdb::tools::copyToDense(*grid, denseBig2, false /* serial */);
         // iterate over the Bigger
        for (openvdb::Coord ijk(bboxBig.min()); ijk[0] <= bboxBig.max()[0]; ++ijk[0]) {
            for (ijk[1] = bboxBig.min()[1]; ijk[1] <= bboxBig.max()[1]; ++ijk[1]) {
                for (ijk[2] = bboxBig.min()[2]; ijk[2] <= bboxBig.max()[2]; ++ijk[2]) {
                    
                    float expected = -1.f; // should never be this
                    if (bboxSmall.isInside(ijk)) {
                        expected = denseSmall.getValue(ijk);
                    } else if (bboxBig.isInside(ijk)) {
                        expected = denseBig.getValue(ijk);
                    }
                    const float& value = denseBig2.getValue(ijk);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, value, 0.0001);
                }
            }
        }
    }
}
#undef BENCHMARK_TEST

// Copyright (c) 2012-2013 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
