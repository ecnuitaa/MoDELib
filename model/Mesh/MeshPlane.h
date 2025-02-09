/* This file is part of MODEL, the Mechanics Of Defect Evolution Library.
 *
 * Copyright (C) 2011 by Giacomo Po <gpo@ucla.edu>.
 *
 * model is distributed without any warranty under the
 * GNU General Public License (GPL) v2 <http://www.gnu.org/licenses/>.
 */

#ifndef model_MeshPlane_H_
#define model_MeshPlane_H_


#include <cfloat>
#include <tuple>
#include <vector>
#include <Eigen/Dense>
#include <SimplicialMesh.h>
#include <PlanarMeshFace.h>
#include <Plane.h>
#include <StaticID.h>
#include <PlaneSegmentIntersection.h>
#include <PlanePlaneIntersection.h>
#include <MeshBoundarySegment.h>
#include <LineLineIntersection.h>
//#include <EmbeddedPolygonTriangulation.h>

namespace model
{
    
    
    template <int dim>
    struct MeshPlane : public StaticID<MeshPlane<dim>>
    /*              */,public Plane<dim>
    {
        
        typedef std::array<long int,dim+3> MeshPlaneKeyType;
        typedef Eigen::Matrix<double,dim,1> VectorDim;
        typedef Eigen::Matrix<double,dim-1,1> VectorLowerDim;
        typedef Eigen::Matrix<double,dim,dim> MatrixDim;
        typedef std::pair<VectorDim,const Simplex<dim,1>* const> RootType;
        typedef std::deque<RootType> RootContainerType;
        typedef std::set<Eigen::Matrix<double,dim,1>,CompareVectorsByComponent<double,dim,float> > UniquePointContainer;
        
        /**********************************************************************/
        static BoundingMeshSegments<dim> getFaceBoundary(const PlanarMeshFace<dim>& face)
        {
            BoundingMeshSegments<dim> temp;
            for(size_t k=0;k<face.convexHull().size();++k)
            {
                const size_t k1(k==face.convexHull().size()-1? 0 : k+1);
                temp.emplace_back(new MeshBoundarySegment<dim>(face.convexHull()[k]->P0,face.convexHull()[k1]->P0,&face));
            }
            return temp;
        }
        
        /**********************************************************************/
        static void checkPlaneIntersections(const BoundingMeshSegments<dim>& temp)
        {
            
            if(temp.size()<3)
            {
                std::cout<<"meshIntersections.size()="<<temp.size()<<std::endl;
                assert(false && "meshIntersections FAILED");
            }
            
            for(const auto& seg : temp)
            {
                assert(!seg->hasZeroLength() && "Plane-Face intersection has zero length");
            }
        }
        
        BoundingMeshSegments<dim> sortMeshIntersections(const BoundingMeshSegments<dim>& mshInt) const
        {
            // Compute midpoints of each segment and center of polygon
            VectorLowerDim center(VectorLowerDim::Zero());
//            std::deque<VectorLowerDim> midPoints;
            for(const auto& segment : mshInt)
            {
//                midPoints.emplace_back(0.5*(this->localPosition(segment->P0)+this->localPosition(segment->P1)));
                center+=0.5*(this->localPosition(segment->P0)+this->localPosition(segment->P1));
            }
            center/=mshInt.size();
            
            // Sort midpoints by angle
            std::map<float,std::shared_ptr<MeshBoundarySegment<dim>>> segmentsByAngle;
            for(const auto& segment : mshInt)
            {
                const VectorLowerDim centerToP0(this->localPosition(segment->P0)-center);
                const float angleP0(std::atan2(centerToP0(1),centerToP0(0)));
                const VectorLowerDim centerToP1(this->localPosition(segment->P1)-center);
                const float angleP1(std::atan2(centerToP1(1),centerToP1(0)));
                
                if(std::fabs(angleP1-angleP0)<=M_PI)
                {// a segment not crossing the branch cut of atan2
                    if(angleP1>angleP0)
                    {// a right-handed segment
                        bool success=segmentsByAngle.emplace(angleP0,segment).second;
                    }
                    else
                    {// a left-handed segment, need to flip it
                        bool success=segmentsByAngle.emplace(angleP1,std::shared_ptr<MeshBoundarySegment<dim>>(new MeshBoundarySegment<dim>(segment->P1,segment->P0,segment->faces))).second;
                    }
                }
                else
                {// a segment crossing the branch cut
                    if(angleP1<0.0)
                    {// P1 below the branch cut, P0 must be above to this is a right-handed segment
                         bool success=segmentsByAngle.emplace(angleP0,segment).second;
                    }
                    else
                    {// P1 above the branch cut, P0 must be below to this is a left-handed segment
                         bool success=segmentsByAngle.emplace(angleP1,std::shared_ptr<MeshBoundarySegment<dim>>(new MeshBoundarySegment<dim>(segment->P1,segment->P0,segment->faces))).second;
                    }
                }
            }
            
            if(segmentsByAngle.size()!=mshInt.size())
            {
                std::cout<<"MeshIntersections.size="<<mshInt.size()<<std::endl;
                std::cout<<"segmentsByAngle="<<segmentsByAngle.size()<<std::endl;
                assert(false && "MeshPlane: Sorting MeshIntersections FAILED.");

            }
            BoundingMeshSegments<dim> temp;
            for(const auto& pair : segmentsByAngle)
            {
                temp.push_back(pair.second);
            }
            return temp;
            
            
        }
        
        const std::pair<int,int> regionIDs;
        const BoundingMeshSegments<dim> meshIntersections;

        /**********************************************************************/
        MeshPlane(const SimplicialMesh<dim>& mesh,
                  const VectorDim& p,
                  const VectorDim& n) :
        /* init */ Plane<dim>(p,n)
        /* init */,regionIDs(-1,-1)
//        /* init */,meshIntersections(mesh,*this)
        /* init */,meshIntersections(sortMeshIntersections(BoundingMeshSegments<dim>(mesh,*this)))
        {/*!\param[in] mesh
          * \param[in] rID the region ID where the plane is defined
          * \param[in] p position of the plane
          * \param[in] n normal to the plane
          * Constructor for plane internal to a mesh region
          */
            checkPlaneIntersections(meshIntersections);
        }
        
        /**********************************************************************/
        MeshPlane(const SimplicialMesh<dim>& mesh,
                  const size_t& rID,
                  const VectorDim& p,
                  const VectorDim& n) :
        /* init */ Plane<dim>(p,n)
        /* init */,regionIDs(rID,rID)
//        /* init */,meshIntersections(mesh,rID,*this)
        /* init */,meshIntersections(sortMeshIntersections(BoundingMeshSegments<dim>(mesh,rID,*this)))
        {/*!\param[in] mesh
          * \param[in] rID the region ID where the plane is defined
          * \param[in] p position of the plane
          * \param[in] n normal to the plane
          * Constructor for plane internal to a mesh region
          */
            checkPlaneIntersections(meshIntersections);
        }
        
        /**********************************************************************/
        MeshPlane(const PlanarMeshFace<dim>& face,
                  const size_t& rID1,
                  const size_t& rID2) :
        /* init */ Plane<dim>(face.asPlane())
        /* init */,regionIDs(rID1,rID2)
//        /* init */ meshIntersections(getFaceBoundary(face)) // WARNING: CALLING meshIntersections with rID1
        /* init */,meshIntersections(sortMeshIntersections(getFaceBoundary(face)))
        {
            checkPlaneIntersections(meshIntersections);
        }
        
        /**********************************************************************/
        template <class T>
        friend T& operator << (T& os, const MeshPlane<dim>& gp)
        {
            for (const auto& x : gp.meshIntersections)
            {
                os<<gp.sID<<" "<<*x;
            }
            return os;
        }
        
    };
    
}
#endif
