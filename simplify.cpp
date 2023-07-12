#include "simplify.h"
#include "Base.h"
#include <queue>
#include<iostream>
void Surface_Simplification(MyMesh& mesh, float ratio)
{
    assert(ratio >= 0 && ratio <= 1);
    int it_num = (1.0f - ratio) * mesh.n_vertices();

 
    //1. Compute the Q matrices for all the initial vertices
    auto Q = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Matrix4d>(mesh);
    auto v = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Vector4d>(mesh);
    auto flag = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, int>(mesh);
    auto p = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Vector4d>(mesh);
    
    for (MyMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        MyMesh::Point fn = mesh.normal(*fit); 
        MyMesh::Point tp = mesh.point(fit->halfedge().to());
        p[*fit][0] = fn[0];
        p[*fit][1] = fn[1];
        p[*fit][2] = fn[2];
        p[*fit][3] = -(fn.dot(tp));
    }
    for (MyMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
    {
        Eigen::Matrix4d mat;
        mat.setZero();
        for (MyMesh::VertexFaceIter vf_it = mesh.vf_iter(*vit); vf_it.is_valid(); ++vf_it)
        {
            mat += p[*vf_it] * p[*vf_it].transpose();
        }
        Q[*vit] = mat;
        /*std::cout << mat << std::endl;
        std::cout << std::endl;*/
        v[*vit][0] = mesh.point(*vit)[0];
        v[*vit][1] = mesh.point(*vit)[1];
        v[*vit][2] = mesh.point(*vit)[2];
        v[*vit][3] = 1.0;
        flag[*vit] = 0;
        
    }
    //std::cout << "caculate curvature done" << std::endl;
    // 2. Select all valid pairs (only vertices in an edge are considered)
    // 3. Compute the optimal contraction target
    std::priority_queue <edge_Collapse_structure, std::vector<edge_Collapse_structure>, std::less<edge_Collapse_structure>> q; 
    for (MyMesh::EdgeIter eit = mesh.edges_begin(); eit != mesh.edges_end(); ++eit)
    {
 
        if (mesh.is_boundary(eit) || mesh.is_boundary(eit->v0()) || mesh.is_boundary(eit->v1()))
        {
            continue;
        }

        Eigen::Matrix4d newQ = Q[eit->v0()] + Q[eit->v1()];
        Eigen::Matrix4d tQ = newQ;
        Eigen::Vector4d b(0.0, 0.0, 0.0, 1.0);
        tQ(3, 0) = 0.0;
        tQ(3, 1) = 0.0;
        tQ(3, 2) = 0.0;
        tQ(3, 3) = 1.0;

        //std::cout << tQ << std::endl;

        Eigen::FullPivLU<Eigen::Matrix4d> lu(tQ);
        Eigen::Vector4d vnew;
        // if is invertible, solve the linear equation
        if (lu.isInvertible())
        {

            vnew = tQ.inverse() * b;
        }
        // else take the midpoint
        else
        {
            vnew = (v[eit->v0()] + v[eit->v1()]) / 2.0f;
        }
        //std::cout << vnew << std::endl;
        edge_Collapse_structure ts;
        ts.hf = eit->halfedge(0);
        /*if (mesh.is_boundary(ts.hf))
        {
            ts.cost = 10.0 * abs(vnew.transpose() * newQ * vnew);
        }
        else*/
            ts.cost = (vnew.transpose() * newQ * vnew);
        if (ts.cost < 0)
        {
            std::cout << "Minus cost:" << ts.cost << std::endl;
            //std::cout << "vnew:" << vnew << std::endl;
            //std::cout << "newQ:" << newQ << std::endl;
        }
           
        MyMesh::Point np(vnew[0], vnew[1], vnew[2]);
        ts.np = np;
        ts.vto = eit->halfedge(0).to();
        ts.vfrom = eit->halfedge(0).from();
        ts.Q_new = newQ;
        q.push(ts);
           
    }
    mesh.request_vertex_status();
    mesh.request_halfedge_normals();
    mesh.request_edge_status();
    mesh.request_face_normals();
    mesh.request_face_status();
    /*if (!mesh.has_face_normals())
    {
        std::cerr << "ERROR: face_normals not available" << "\n";
    }*/
    std::cout << "simplify begin..." << std::endl;
    int i = 0;
    while (i < it_num)
    {
        edge_Collapse_structure s = q.top();
        q.pop();
 
        if (mesh.status(s.vfrom).deleted() || mesh.status(s.vto).deleted())
            continue;
        if (s.vto_flag != flag[s.vto] || s.vfrom_flag != flag[s.vfrom])
            continue;
        MyMesh::VertexHandle tvh;
        MyMesh::HalfedgeHandle ophf = mesh.opposite_halfedge_handle(s.hf);
        if (mesh.is_collapse_ok(s.hf) && !mesh.is_boundary(s.hf))
        {
           
            mesh.collapse(s.hf);
            tvh = s.vto;
            flag[s.vto] ++;
            flag[s.vfrom] ++;    
            
        }

        
        else if (mesh.is_collapse_ok(ophf) && !mesh.is_boundary(ophf))
        {
          
            mesh.collapse(ophf);
            tvh = s.vto;
            flag[s.vto] ++;
            flag[s.vfrom] ++;

        }
        else
        {
            continue;
        }

        mesh.set_point(tvh, s.np);
        Q[tvh] = s.Q_new;
        v[tvh][0] = s.np[0];
        v[tvh][1] = s.np[1];
        v[tvh][2] = s.np[2];
        v[tvh][3] = 1.0f;
        for (MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(tvh); vh_it.is_valid(); ++vh_it)
        {
            MyMesh::VertexHandle tt = vh_it->to();
            Eigen::Matrix4d newQ = s.Q_new + Q[tt];
            Eigen::Matrix4d tQ = newQ;
            Eigen::Vector4d b(0.0, 0.0, 0.0, 1.0);
            tQ(3, 0) = 0.0;
            tQ(3, 1) = 0.0;
            tQ(3, 2) = 0.0;
            tQ(3, 3) = 1.0;
            Eigen::FullPivLU<Eigen::Matrix4d> lu(tQ);
            Eigen::Vector4d vnew;
            // if is invertible, solve the linear equation
            if (lu.isInvertible())
            {
                vnew = tQ.inverse() * b;
            }
            // else take the midpoint
            else
            {
                vnew = (v[tvh] + v[tt]) / 2.0f;
            }
            //std::cout << vnew << std::endl;
            edge_Collapse_structure ts;
            ts.hf = *vh_it;
            ts.cost = (vnew.transpose() * newQ * vnew);
            MyMesh::Point np(vnew[0], vnew[1], vnew[2]);
            ts.np = np;
            ts.vto = tt;           
            ts.vto_flag = flag[tt];
            ts.vfrom = tvh;
            ts.vfrom_flag = flag[tvh];
            ts.Q_new = newQ;
            q.push(ts);
            
        }
        i++;
        
    }
    mesh.garbage_collection();



}
